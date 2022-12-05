#pragma once

#include "soro/base/soro_types.h"

#include "soro/utls/container/it_range.h"

namespace soro::utls {

// removes elements from the vector so that the vector only retains the elements
// in the range [from, to).
// precondition: 0 <= {from, to} <= v.size()
template <typename T>
constexpr void slice(soro::vector<T>& v, std::size_t const from,
                     std::size_t const to) {
  v.erase(std::begin(v) + static_cast<std::ptrdiff_t>(to), std::end(v));
  v.erase(std::begin(v), std::begin(v) + static_cast<std::ptrdiff_t>(from));
}

// returns a vector with elements from the given vector so that the
// returned vector only retains the elements in the range [from, to).
template <typename T>
[[nodiscard]] constexpr soro::vector<T> slice(soro::vector<T> const& v,
                                              std::size_t const from,
                                              std::size_t const to) {
  soro::vector<T> slice;
  slice.reserve(static_cast<soro::size_t>(to - from));

  std::copy(std::cbegin(v) + static_cast<std::ptrdiff_t>(from),
            std::cbegin(v) + static_cast<std::ptrdiff_t>(to),
            std::back_inserter(slice));

  return slice;
}

// removes elements from the vector so that the vector only retains the elements
// in the range [from, to).
// if from > v.size(), then from is set to 0
// if to > v.size(), then to is set to v.size()
template <typename T>
constexpr void slice_clasp(soro::vector<T>& v, std::size_t from,
                           std::size_t to) {
  from = from > v.size() ? 0 : from;
  to = to > v.size() ? v.size() : to;

  slice(v, from, to);
}

// returns a vector so that its only contains the elements in the
// range [from, to).
// if from > v.size(), then from is set to 0
// if to > v.size(), then to is set to v.size()
template <typename T>
[[nodiscard]] constexpr soro::vector<T> slice_clasp(soro::vector<T> const& v,
                                                    std::size_t from,
                                                    std::size_t to) {
  from = from > v.size() ? 0 : from;
  to = to > v.size() ? v.size() : to;

  return slice(v, from, to);
}

template <typename T>
constexpr auto slice_range(soro::vector<T> const& v, std::size_t const from,
                           std::size_t const to) {
  return make_range(std::begin(v) + static_cast<std::ptrdiff_t>(from),
                    std::begin(v) + static_cast<std::ptrdiff_t>(to));
}

template <typename T>
constexpr auto slice_clasp_range(soro::vector<T> const& v, std::size_t from,
                                 std::size_t to) {
  from = from > v.size() ? 0 : from;
  to = to > v.size() ? v.size() : to;

  return slice_range(v, from, to);
}

}  // namespace soro::utls
