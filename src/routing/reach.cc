#include "nigiri/routing/reach.h"

#include <nigiri/routing/raptor_search.h>
#include <utl/parallel_for.h>
#include <utl/progress_tracker.h>

namespace nigiri::routing {

std::vector<location_idx_t> reach::find_sample_locations_from_start(
    location_idx_t start, interval<date::sys_days> search_interval) {
  using namespace date;
  // run raptor search in one to all mode
  auto const q = query{.start_time_ = search_interval,
                       .start_ = {{start, 0_minutes, 0U}},
                       .start_match_mode_ = location_match_mode::kEquivalent,
                       .dest_match_mode_ = location_match_mode::kEquivalent,
                       .one_to_all_ = true};

  raptor_search(tt_, nullptr, search_state, raptor_state, q,
                direction::kForward);

  // round times for each number of transfers
  auto const round_times = raptor_state.get_round_times<0>();
  auto const n_legs = round_times.n_rows_;
  [[maybe_unused]] auto cmp = [](auto a, auto b) {
    return a.second > b.second;
  };
  std::set<std::pair<location_idx_t, short>, decltype(cmp)> ordered_times;

  for (size_t i = 0; i < round_times.n_columns_; ++i) {
    auto min = std::numeric_limits<short>::max();
    for (size_t j = 0; j < n_legs; ++j) {
      min = std::min(min, round_times[j][i][0]);
    }

    if (min == std::numeric_limits<short>::max()) continue;
    ordered_times.emplace(location_idx_t{i}, min);
  }

  std::vector boundary_stations{location_idx_t(ordered_times.begin()->first)};

  // remove locations that are within 50km of an existing boundary station
  for (auto const& [location, time] : ordered_times) {
    auto const current_coord = tt_.locations_.coordinates_[location];

    auto in_radius = false;
    for (const auto& boundary_station : boundary_stations) {
      auto const previous_coord = tt_.locations_.coordinates_[boundary_station];
      if (distance(previous_coord, current_coord) <= minDistBoundaryStations) {
        in_radius = true;
        break;
      }
    }
    if (!in_radius) {
      boundary_stations.push_back(location);
    }
  }

  return boundary_stations;
}
}  // namespace nigiri::routing