#pragma once

#include "soro/base/soro_types.h"

#include "soro/infrastructure/interlocking/get_interlocking_subsystem.h"

namespace soro::infra {

using exclusion_set_id = uint32_t;

soro::vector<soro::vector<exclusion_set_id>>
get_interlocking_route_exclusion_sets(infrastructure_t const& infra);

/*
 *  Exclusion paths are used to partition the infrastructure into
 *  paths that can be used to compose station routes as well as interlocking
 *  routes.
 *
 *  The reason for this model is that we can determine the exclusions w.r.t
 *  all exclusion paths and from them infer all station route and interlocking
 *  exclusions.
 *
 *  Therefore, an exclusion path always starts and end on one of the following:
 *  - Main Signal
 *  - Border
 *  - Track End
 *  - Halt (if it is used by at least one station route as its start or end)
 *
 *  Since we only need this model for determining the interlocking exclusions
 *  the nodes will be sorted.
 */
struct exclusion_path {
  using id = uint32_t;

  static constexpr id INVALID = std::numeric_limits<id>::max();

  id id_{INVALID};

  // contains all section elements and the head/tail track elements
  // is sorted
  std::vector<element_id> elements_;
};

std::vector<std::vector<exclusion_path::id>> get_exclusion_path_exclusions(
    infrastructure const& infra);

soro::vector<soro::vector<interlocking_route::id>>
get_interlocking_route_exclusions(interlocking_subsystem const& irs,
                                  infrastructure_t const& infra);

soro::vector<element_ptr> get_exclusion_elements(interlocking_route const& ssr,
                                                 infrastructure_t const& iss);

namespace detail {

std::vector<std::vector<station_route::id>> get_station_route_exclusions(
    infrastructure_t const& infra);

}  // namespace detail

}  // namespace soro::infra