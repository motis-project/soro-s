#include "soro/infrastructure/exclusion/get_exclusion_graph.h"

#include <algorithm>
#include <iterator>
#include <thread>
#include <utility>
#include <vector>

#include "utl/concat.h"
#include "utl/erase_duplicates.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/algo/multi_set_merge.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/is_sorted.h"
#include "soro/utls/std_wrapper/set_difference.h"

#include "soro/infrastructure/exclusion/exclusion_elements.h"
#include "soro/infrastructure/exclusion/exclusion_graph.h"
#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

exclusion_graph get_exclusion_graph(
    soro::vector_map<ir_id, element::ids> const& closed_exclusion_elements,
    soro::vector_map<element::id, interlocking_route::ids> const&
        closed_irs_using,
    infrastructure const& infra) {
  utl::scoped_timer const timer("creating exclusion graph");

  utls::sasserts([&]() {
    for (auto const& irs_using_element : closed_irs_using) {
      utls::sassert(utls::is_sorted(irs_using_element));
    }

    for (auto const& ir_elements : closed_exclusion_elements) {
      utls::sassert(utls::is_sorted(ir_elements));
    }
  });

  auto const ir_count = closed_exclusion_elements.size();

  exclusion_graph g;
  g.nodes_.resize(ir_count);

  utl::parallel_for_run(ir_count, [&](auto&& ir_id_large) {
    auto const ir_id = static_cast<interlocking_route::id>(ir_id_large);

    auto const ranges =
        soro::to_vec(closed_exclusion_elements[ir_id], [&](auto&& e_id) {
          auto const& irs_using = closed_irs_using[e_id];
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

// for every element returns the interlocking routes using that element
soro::vector_map<element::id, interlocking_route::ids> get_closed_irs_using(
    soro::vector_map<interlocking_route::id, element::ids> const&
        closed_exclusion_elements,
    soro::size_t const element_count) {
  utl::scoped_timer const timer("generating element used by mapping");

  auto const ir_count = closed_exclusion_elements.size();

  using result_t = soro::vector_map<element::id, interlocking_route::ids>;
  result_t irs_using(element_count);

  // Set up the thread results ...
  auto const thread_count = std::thread::hardware_concurrency();
  soro::vector<result_t> thread_results(thread_count, result_t(element_count));

  // ... thread lambda ...

  auto const generate_used_by = [&](soro::size_t const t_id) {
    auto const batch_size = ir_count / thread_count;
    auto const from = static_cast<interlocking_route::id>(t_id * batch_size);
    auto const to =
        t_id == thread_count - 1 ? ir_count : ((t_id + 1) * batch_size);

    for (interlocking_route::id ir_id = from; ir_id < to; ++ir_id) {
      for (auto const e_id : closed_exclusion_elements[ir_id]) {
        thread_results[t_id][e_id].emplace_back(ir_id);
      }
    }
  };

  // ... threads.

  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (auto t = 0U; t < thread_count; ++t) {
    threads.emplace_back(generate_used_by, t);
  }

  std::for_each(begin(threads), end(threads), [](auto& t) { t.join(); });

  timer.print("threads finished");

  // Combine all thread results

  utl::parallel_for_run(irs_using.size(), [&](auto&& id) {
    auto const element_id = element::id{utls::narrow<element::id::value_t>(id)};
    for (auto t = 0U; t < thread_count; ++t) {
      utl::concat(irs_using[element_id], thread_results[t][element_id]);
    }
  });

  timer.print("thread results combined");

  utl::parallel_for_run(irs_using.size(), [&](auto&& id) {
    auto const element_id = element::id{utls::narrow<element::id::value_t>(id)};
    utl::erase_duplicates(irs_using[element_id]);
  });

  timer.print("duplicates removed");

  utls::ensure(irs_using.size() == element_count,
               "Mapping has to exist for every element");

  return irs_using;
}

exclusion_graph get_exclusion_graph(infrastructure const& infra) {
  auto const closed_exclusion_elements = get_closed_exclusion_elements(infra);

  auto const closed_irs_using = get_closed_irs_using(
      closed_exclusion_elements, infra->graph_.elements_.size());

  return get_exclusion_graph(closed_exclusion_elements, closed_irs_using,
                             infra);
}

}  // namespace soro::infra
