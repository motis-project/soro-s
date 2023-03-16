#include "soro/infrastructure/exclusion/get_exclusion_graph.h"

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/utls/algo/multi_set_merge.h"
#include "soro/utls/std_wrapper/is_sorted.h"
#include "soro/utls/std_wrapper/set_difference.h"

namespace soro::infra {

exclusion_graph get_exclusion_graph(
    soro::vector<element::ids> const& closed_exclusion_elements,
    soro::vector<interlocking_route::ids> const& closed_element_used_by,
    infrastructure const& infra) {
  utl::scoped_timer const timer("creating exclusion graph");

  utls::sasserts([&]() {
    for (auto const& irs_using_element : closed_element_used_by) {
      utls::sassert(utls::is_sorted(irs_using_element));
    }

    for (auto const& ir_elements : closed_exclusion_elements) {
      utls::sassert(utls::is_sorted(ir_elements));
    }
  });

  auto const ir_count =
      static_cast<interlocking_route::id>(closed_exclusion_elements.size());

  exclusion_graph g;
  g.nodes_.resize(ir_count);

  utl::parallel_for_run(ir_count, [&](auto&& ir_id) {
    auto const ranges =
        soro::to_vec(closed_exclusion_elements[ir_id], [&](auto&& e_id) {
          auto const& irs_using = closed_element_used_by[e_id];
          return std::pair(std::cbegin(irs_using), std::cend(irs_using));
        });

    auto const merged_set =
        utls::multi_set_merge<interlocking_route::ids>(ranges);

    // remove all other IRs that are direct predecessors/successors
    auto const& ir = infra->interlocking_.routes_[ir_id];
    interlocking_route::ids remove =
        infra->interlocking_.starting_at_[ir.last_node(infra)->id_];
    utl::concat(remove,
                infra->interlocking_.ending_at_[ir.first_node(infra)->id_]);
    utl::erase_duplicates(remove);

    interlocking_route::ids finished_set;
    finished_set.reserve(merged_set.size());
    utls::set_difference(merged_set, remove, std::back_inserter(finished_set));

    g.nodes_[ir_id] = make_exclusion_set(finished_set);
  });

  return g;
}

}  // namespace soro::infra
