#pragma once

#include <algorithm>
#include <iterator>

namespace soro::utls {

/*
 * Sorts both containers and returns the intersection of both.
 * Containers will be modified by reordering the elements.
 */

template <typename Container>
inline Container sort_and_intersect(Container& c1, Container& c2) {
  std::sort(std::begin(c1), std::end(c1));
  std::sort(std::begin(c2), std::end(c2));

  Container result;
  result.reserve(c1.size());

  std::set_intersection(std::cbegin(c1), std::cend(c1), std::cbegin(c2),
                        std::cend(c2), std::back_inserter(result));

  return result;
}

}  // namespace soro::utls
