#pragma once

#include "nigiri/routing/journey.h"
#include "nigiri/types.h"
#include <vector>
#include "raptor/raptor_state.h"
#include "search.h"

namespace nigiri {
struct timetable;
}

namespace nigiri::routing {
struct reach_info {
  reach_info();

  bool valid() const noexcept {
    return start_end_ != location_idx_t::invalid();
  }

  void update(float const new_reach,
              routing::journey const&,
              location_idx_t const start_end,
              location_idx_t const stop_in_route);

  float reach_{-1.0};
  routing::journey j_;
  location_idx_t start_end_{location_idx_t::invalid()};
  location_idx_t stop_in_route_{location_idx_t::invalid()};
};

class reach {
  auto constexpr static minDistBoundaryStations = 50'000;

public:
  reach(timetable const& tt,
        location_idx_t const start,
        interval<date::sys_days> const search_interval)
      : tt_(tt) {
    sample_locations = find_sample_locations_from_start(start, search_interval);
  }

  std::vector<reach_info> compute_reach_values(
      interval<date::sys_days> const search_interval);

  void reach_values_for_source(date::sys_days base_day,
                               interval<date::sys_days> search_interval,
                               location_idx_t l,
                               std::vector<reach_info>& route_reachs);

  /**
   * Find a set of locations that lie at the boundary of the graph of locations
   * reachable from the start location.
   * @param tt timetable
   * @param start start location
   * @param search_interval time interval to search for locations
   * @return the stations on the boundary
   */
  std::vector<location_idx_t> find_sample_locations_from_start(
      location_idx_t start, interval<date::sys_days> search_interval);

private:
  const timetable tt_;
  std::vector<location_idx_t> sample_locations;
  search_state search_state;
  raptor_state raptor_state;
};
}  // namespace nigiri::routing