#include "soro/ordering/remove_transitive_edges.h"

#include <string>

#include "utl/concat.h"
#include "utl/erase.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/parallel_for.h"

#include "soro/ordering/get_transitive_dependency_edges.h"
#include "soro/ordering/graph.h"

namespace soro::ordering {

soro::vector<graph::edge> get_transitive_dependency_edges(
    soro::vector<soro::vector<graph::node::id>> const& outgoing_edges,
    graph const& og) {
  utl::scoped_timer const timer("determining transitive edges");

  using result_t = soro::vector<graph::edge>;

  auto const do_work = [&](soro::size_t const job_id) {
    return get_transitive_dependency_edges(graph::node::id{job_id},
                                           outgoing_edges, og);
  };

  auto const combine_work = [&](auto&& result, auto&& partial) {
    utl::concat(result, partial);
  };

  return utls::parallel_for<result_t>(outgoing_edges, do_work, combine_work);
}

void erase_transitive_edges(
    soro::vector<graph::edge> const& transitive_edges,
    soro::vector<soro::vector<graph::node::id>>& outgoing_edges) {
  utl::scoped_timer const timer("erasing " +
                                std::to_string(transitive_edges.size()) +
                                " transitive edges");

  for (auto const& edge : transitive_edges) {
    utl::erase(outgoing_edges[as_val(edge.from_)], edge.to_);
  }
}

void remove_transitive_dependency_edges(
    soro::vector<soro::vector<graph::node::id>>& outgoing, graph const& og) {
  utl::scoped_timer const timer("removing transitive dependency edges");

  erase_transitive_edges(get_transitive_dependency_edges(outgoing, og),
                         outgoing);
}

}  // namespace soro::ordering
