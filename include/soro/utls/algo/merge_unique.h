#pragma once

#include <algorithm>

#include "utl/verify.h"

namespace soro::utls {

/*
 * Returns a new container containing a set of all elements in the union of
 * both passed containers.
 *
 * Requires the containers to be sorted!
 */

template <typename Container>
inline Container merge_unique(Container const& c1, Container const& c2) {
  utl::verify(std::is_sorted(std::cbegin(c1), std::cend(c1)),
              "Container c1 is not sorted!");
  utl::verify(std::is_sorted(std::cbegin(c2), std::cend(c2)),
              "Container c2 is not sorted!");

  Container result;
  result.reserve(c1.size() + c2.size());

  std::merge(std::cbegin(c1), std::cend(c1), std::cbegin(c2), std::cend(c2),
             std::back_inserter(result));

  result.erase(std::unique(std::begin(result), std::end(result)),
               std::end(result));

  return result;
}

}  // namespace soro::utls