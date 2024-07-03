#include "soro/infrastructure/exclusion/get_exclusion.h"

#include <filesystem>

#include "utl/enumerate.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/infrastructure/exclusion/exclusion.h"
#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/exclusion/read_cliques.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/infrastructure_options.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

soro::vector_map<interlocking_route::id, exclusion_set::ids>
get_irs_to_exclusion_sets(
    soro::vector<interlocking_route::ids> const& exclusion_sets,
    soro::size_t const interlocking_route_count) {
  utl::scoped_timer const timer("generating irs to exclusion sets mapping");

  soro::vector_map<interlocking_route::id, exclusion_set::ids>
      irs_to_exclusion_sets(interlocking_route_count);

  for (auto const [id, exclusion_set] :
       utl::enumerate<exclusion_set::id>(exclusion_sets)) {
    for (auto const ir_id : exclusion_set) {
      irs_to_exclusion_sets[ir_id].emplace_back(id);
    }
  }

  return irs_to_exclusion_sets;
}

exclusion get_exclusion(infrastructure_t const& infra_t,
                        std::filesystem::path const& clique_path,
                        option<exclusion_sets_tag> const exclusion_sets) {
  infrastructure const infra(&infra_t);

  exclusion ex;

  if (exclusion_sets) {
    ex.exclusion_sets_ = read_cliques(clique_path);

    ex.irs_to_exclusion_sets_ = get_irs_to_exclusion_sets(
        ex.exclusion_sets_, infra->interlocking_.routes_.size());
  }

  return ex;
}

}  // namespace soro::infra
