#pragma once

#include <algorithm>

namespace soro::utls {

namespace detail {

using std::begin;
using std::end;

template <typename Container, typename Element>
void erase(Container& c, Element&& e) {
  c.erase(std::remove(begin(c), end(c), std::forward<Element>(e)));
}

}  // namespace detail

template <typename Container, typename Element>
constexpr void erase(Container&& c, Element&& t) {
  return detail::erase(std::forward<Container>(c), std::forward<Element>(t));
}

}  // namespace soro::utls
