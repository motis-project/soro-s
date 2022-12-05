#pragma once

#include <tuple>

namespace soro::utls {

template <typename Iterator>
struct it_range {
  it_range(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

  operator std::tuple<Iterator&, Iterator&>() {  // NOLINT
    return {begin_, end_};
  }

  constexpr Iterator begin() const noexcept { return begin_; }
  constexpr Iterator end() const noexcept { return end_; }

  Iterator begin_, end_;
};

template <typename Iterator>
constexpr auto make_range(Iterator begin, Iterator end) noexcept {
  return it_range<Iterator>(begin, end);
}

template <typename Container>
constexpr auto make_range(Container const& c) {
  return it_range(std::begin(c), std::end(c));
}

}  // namespace soro::utls
