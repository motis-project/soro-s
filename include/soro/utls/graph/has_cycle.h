#pragma once

#include <ranges>
#include <vector>

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/any_of.h"

namespace soro::utls {

template <typename Nodes, typename GetNeighbours>
bool has_cycle(Nodes&& nodes, GetNeighbours&& get_neighbours) {

  std::vector<bool> visited(nodes.size());
  std::vector<bool> finished(nodes.size());

  auto const cycle_detection = [&](std::integral auto const n_outer) {
    auto const cycle_detection_impl = [&](std::integral auto const n,
                                          auto&& impl) -> bool {
      if (finished[n]) {
        return false;
      }

      if (visited[n]) {
        return true;
      }

      visited[n] = true;

      for (auto const& neighbour : get_neighbours(nodes, n)) {
        if (impl(neighbour, impl)) {
          return true;
        }
      }

      finished[n] = true;

      return false;
    };

    return cycle_detection_impl(n_outer, cycle_detection_impl);
  };

  auto const found_cycle =
      utls::any_of(std::ranges::views::iota(std::size_t{0}, nodes.size()),
                   [&](auto&& node) { return cycle_detection(node); });

  utls::ensure(found_cycle || (utls::all_of(visited) && utls::all_of(finished)),
               "all nodes must be visited & finished");

  return found_cycle;
}

}  // namespace soro::utls