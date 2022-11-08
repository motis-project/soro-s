#pragma once

#include <tuple>

namespace soro::utls {

template <typename Iterator>
struct iterator_range {
  iterator_range(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

  operator std::tuple<Iterator&, Iterator&>() {  // NOLINT
    return {begin_, end_};
  }

  constexpr Iterator begin() const noexcept { return begin_; }
  constexpr Iterator end() const noexcept { return end_; }

  Iterator begin_, end_;
};

template <typename Iterator>
constexpr auto make_range(Iterator begin, Iterator end) noexcept {
  return iterator_range<Iterator>(begin, end);
}

}  // namespace soro::utls
