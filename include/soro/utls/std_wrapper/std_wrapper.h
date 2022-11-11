#pragma once

#include <numeric>

#include "utl/verify.h"

namespace soro::utls {

template <
    typename Container, typename ValueType,
    std::enable_if_t<!std::is_same_v<Container, std::string>, bool> = true>
inline bool contains(Container const& c, ValueType const& e) {
  return std::find(std::cbegin(c), std::cend(c), e) != std::cend(c);
}

template <typename Container, typename Pred>
inline bool contains_if(Container const& c, Pred const& pred) {
  return std::find_if(std::cbegin(c), std::cend(c), pred) != std::cend(c);
}

template <typename ValueType>
inline auto copy(std::vector<ValueType> const& v, size_t const from,
                 size_t const to) {
  std::vector<ValueType> dst;
  dst.reserve(1 + (to - from));
  std::copy(std::next(std::cbegin(v), static_cast<int32_t>(from)),
            std::next(std::cbegin(v), static_cast<int32_t>(to + 1)),
            std::back_inserter(dst));
  return dst;
}

template <typename C, typename DestIterator>
constexpr auto copy(C const& c, DestIterator&& dst) {
  std::copy(std::cbegin(c), std::cend(c), dst);
}

template <typename Container, typename Pred>
inline auto copy_if(Container const& c, Pred const&& filter) {
  std::vector<typename Container::value_type> v(c.size());
  auto copy_it =
      std::copy_if(std::cbegin(c), std::cend(c), std::begin(v), filter);
  v.erase(copy_it, std::end(v));
  v.shrink_to_fit();
  return v;
}

template <typename Container, typename Type>
inline auto find(Container const& c, Type const& t) {
  return std::find(std::cbegin(c), std::cend(c), t);
}

template <typename Container, typename Pred>
inline auto find_if(Container const& c, Pred const& pred) {
  return std::find_if(std::cbegin(c), std::cend(c), pred);
}

template <typename Container, typename Type>
inline auto find_position(Container const& c, Type const& t) {
  return static_cast<std::size_t>(std::distance(
      std::cbegin(c), std::find(std::cbegin(c), std::cend(c), t)));
}

template <typename Container, typename Pred>
inline auto find_if_position(Container const& c, Pred const& pred) {
  return static_cast<std::size_t>(std::distance(
      std::cbegin(c), std::find_if(std::cbegin(c), std::cend(c), pred)));
}

template <typename It, typename Pred>
inline auto find_if_position(It&& begin, It&& end, Pred&& pred) {
  return static_cast<std::size_t>(
      std::distance(begin, std::find_if(begin, end, pred)));
}

template <typename Container, typename T>
inline auto reverse_find_position(Container const& c, T const& v) {
  return static_cast<std::size_t>(std::distance(
             std::find(std::crbegin(c), std::crend(c), v), std::crend(c))) -
         1;
}

template <typename Container, typename Pred>
inline auto reverse_find_if(Container const& c, Pred const& pred) {
  return std::find_if(std::crbegin(c), std::crend(c), pred);
}

template <typename Container, typename Pred>
inline auto reverse_find_if(Container& c, Pred&& pred) {
  return std::find_if(std::rbegin(c), std::rend(c), pred);
}

template <typename Container, typename Pred>
inline auto reverse_find_if_position(Container const& c, Pred&& pred) {
  return static_cast<std::size_t>(
             std::distance(std::find_if(std::crbegin(c), std::crend(c), pred),
                           std::crend(c))) -
         1;
}

template <typename Container, typename Pred>
inline auto for_each(Container const& c, Pred const& pred) {
  return std::for_each(std::cbegin(c), std::cend(c), pred);
}

template <typename Container, typename Pred>
inline auto for_each(Container& c, Pred const& pred) {
  return for_each(c, pred);
}

template <typename Container>
inline void unique_erase(Container& c) {
  c.erase(std::unique(std::begin(c), std::end(c)), std::end(c));
}

template <typename Container>
inline auto sort(Container& c) {
  return std::sort(std::begin(c), std::end(c));
}

template <typename Container, typename Pred>
inline auto sort(Container& c, Pred const&& pred) {
  return std::sort(std::begin(c), std::end(c), pred);
}

template <typename C1, typename C2>
inline void append(C1& dest, C2 const& src) {
  dest.reserve(dest.size() + src.size());
  dest.insert(std::end(dest), std::begin(src), std::end(src));
}

template <typename C1, typename C2>
[[nodiscard]] constexpr auto concat(C1 const& c1, C2 const& c2) {
  auto result = c1;
  result.reserve(c1.size() + c2.size());
  result.insert(std::end(c1), std::begin(c2), std::end(c2));
  return result;
}

template <typename C1, typename C2>
inline void append_move(C1& dest, C2&& src) {
  if (dest.empty()) {
    dest = std::forward<C2>(src);
  } else {
    dest.reserve(dest.size() + src.size());
    dest.insert(std::end(dest), std::make_move_iterator(std::begin(src)),
                std::make_move_iterator(std::end(src)));
  }

  src.clear();
}

template <typename Iteratable, typename Pred>
inline auto any_of(Iteratable const& i, Pred&& p) {
  return std::any_of(std::cbegin(i), std::cend(i), p);
}

template <typename Iteratable, typename Pred>
constexpr auto all_of(Iteratable&& i, Pred&& p) {
  return std::all_of(std::begin(i), std::end(i), p);
}

template <typename Iterable, typename T>
constexpr auto sum(Iterable const& i, T init) {
  return std::accumulate(std::cbegin(i), std::cend(i), init);
}

template <typename Iterable, typename Pred>
constexpr std::size_t count_if(Iterable const& i, Pred&& p) {
  return static_cast<std::size_t>(
      std::count_if(std::cbegin(i), std::cend(i), p));
}

template <typename Iterable, typename T>
constexpr std::size_t count(Iterable const& i, T const& v) {
  return static_cast<std::size_t>(std::count(std::cbegin(i), std::cend(i), v));
}

}  // namespace soro::utls
