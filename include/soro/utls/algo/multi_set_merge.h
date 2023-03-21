#pragma once

#include <queue>

#include "utl/erase_if.h"

#include "soro/base/soro_types.h"

namespace soro::utls {

// merges multiple sets into a single one
// requires the sets to be sorted
template <typename Result, typename Start, typename End>
Result multi_set_merge(soro::vector<std::pair<Start, End>> ranges) {
  using range = std::pair<Start, End>;

  // remove empty ranges
  utl::erase_if(ranges, [](auto&& r) { return r.first == r.second; });

  if (ranges.empty()) {
    return {};
  }

  if (ranges.size() == 1) {
    return Result{ranges.front().first, ranges.front().second};
  }

  Result result;

  if constexpr (std::contiguous_iterator<Start>) {
    auto const max = std::max_element(
        std::begin(ranges), std::end(ranges), [](auto&& r1, auto&& r2) {
          return std::distance(r1.first, r1.second) <
                 std::distance(r2.first, r2.second);
        });

    result.reserve(static_cast<typename Result::size_type>(
        distance(max->first, max->second)));
  }

  auto const cmp = [&](auto&& r1, auto&& r2) { return *r1.first > *r2.first; };
  std::priority_queue<range, std::vector<range>, decltype(cmp)> queue(
      std::begin(ranges), std::end(ranges), cmp);

  // we don't want to check if result is empty later on
  result.emplace_back(*queue.top().first);

  while (!queue.empty()) {
    auto current = queue.top();
    queue.pop();

    if (*current.first > result.back()) {
      result.emplace_back(*current.first);
    }

    while (current.first != current.second && *current.first <= result.back()) {
      ++(current.first);
    }

    if (current.first != current.second) {
      queue.emplace(current);
    }
  }

  return result;
}

}  // namespace soro::utls
