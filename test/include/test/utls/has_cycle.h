#pragma once

#include "soro/base/soro_types.h"

#include "soro/utls/print_progress.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/any_of.h"

namespace soro::test {

namespace detail {

template <typename NodeId, typename Graph>
bool starts_cycle(NodeId const& n, Graph const& g, std::vector<bool>& visited,
                  std::vector<bool>& finished) {
  auto const idx = as_val(n);

  if (finished[idx]) return false;

  if (visited[idx]) return true;

  visited[idx] = true;

  for (auto const& neighbour : g.out(n)) {
    if (starts_cycle(neighbour, g, visited, finished)) return true;
  }

  finished[idx] = true;

  return false;
}

template <typename Graph>
bool has_cycle(Graph const& g) {
  std::vector<bool> visited(g.nodes_.size(), false);
  std::vector<bool> finished(g.nodes_.size(), false);

  auto const has_cycle = utls::any_of(g.nodes_, [&](auto&& n) {
    utls::print_progress("checking for cycles", g.nodes_);
    return starts_cycle(n.get_id(g), g, visited, finished);
  });

  utls::ensure(has_cycle || (utls::all_of(visited) && utls::all_of(finished)),
               "all nodes must be visited & finished");

  return has_cycle;
}

}  // namespace detail

template <typename Graph>
[[nodiscard]] constexpr bool has_cycle(Graph const& g) {
  return detail::has_cycle(g);
}

}  // namespace soro::test