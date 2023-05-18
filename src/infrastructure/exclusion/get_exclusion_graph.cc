#include "soro/infrastructure/exclusion/get_exclusion_graph.h"

#include "utl/concat.h"
#include "utl/enumerate.h"
#include "utl/erase_duplicates.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/utls/algo/multi_set_merge.h"
#include "soro/utls/std_wrapper/is_sorted.h"
#include "soro/utls/std_wrapper/set_difference.h"

namespace soro::infra {

utls::optional<element_id> get_route_eotd(interlocking_route const& from,
                                          interlocking_route const& to) {
  utls::sassert(false, "not implemented");

  std::ignore = from;
  std::ignore = to;
  return {};
}

soro::vector<utls::offset_container<soro::vector<exclusion_data>>>
get_exclusion_data(soro::vector<exclusion_set> const& nodes,
                   infrastructure const& infra) {

  soro::vector<utls::offset_container<soro::vector<exclusion_data>>> result;
  result.reserve(infra->interlocking_.routes_.size());

  for (auto const& [from_id, set] : utl::enumerate(nodes)) {
    auto const& from_ir = infra->interlocking_.routes_[from_id];

    soro::vector<exclusion_data> data;

    for (auto const to_id : set) {
      auto const& to_ir = infra->interlocking_.routes_[to_id];

      data.emplace_back(get_route_eotd(from_ir, to_ir));
    }

    //    result.emplace_back(std::move(exclusion_data));
  }

  utls::sassert(false, "implement creation of exclusion data");

  return result;
}

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

  utl::parallel_for_run(ir_count, [&](auto&& ir_id_large) {
    auto const ir_id = static_cast<interlocking_route::id>(ir_id_large);

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

  g.data_ = get_exclusion_data(g.nodes_, infra);

  return g;
}

}  // namespace soro::infra
