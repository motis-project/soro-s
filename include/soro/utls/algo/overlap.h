#pragma once

#include <cassert>
#include <numeric>

#include "utl/verify.h"

#include "soro/utls/concepts/iterable_helpers.h"

namespace soro::utls {

struct strong_idx {
  auto& operator++() {
    ++idx_;
    return *this;
  }

  size_t idx_;
};

struct strong_idx1 : strong_idx {};
struct strong_idx2 : strong_idx {};

template <typename It, typename IdxType = size_t>
auto get_permutation_index(It const& begin, It const& end) {
  std::vector<IdxType> permutation_index(
      static_cast<size_t>(std::distance(begin, end)));
  std::iota(std::begin(permutation_index), std::end(permutation_index),
            IdxType{0});

  std::sort(std::begin(permutation_index), std::end(permutation_index),
            [&](auto const& i1, auto const& i2) {
              if constexpr (std::is_integral_v<IdxType>) {
                return *(begin + static_cast<std::ptrdiff_t>(i1)) <
                       *(begin + static_cast<std::ptrdiff_t>(i2));
              } else {
                return *(begin + static_cast<std::ptrdiff_t>(i1.idx_)) <
                       *(begin + static_cast<std::ptrdiff_t>(i2.idx_));
              }
            });

  return permutation_index;
}

template <typename Container, typename IdxType = size_t>
auto get_permutation_index(Container const& c) {
  return get_permutation_index<decltype(std::cbegin(c)), IdxType>(
      std::cbegin(c), std::end(c));
}

template <typename It>
struct sorted_wrapper {
  sorted_wrapper(It const& begin, std::vector<size_t> const& perm_idx)
      : begin_{begin}, perm_idx_{perm_idx} {}

  auto const& operator[](size_t const idx) const {
    return *(begin_ + static_cast<std::ptrdiff_t>(perm_idx_[idx]));
  }
  auto size() const { return perm_idx_.size(); }

  It begin_;
  std::vector<size_t> const& perm_idx_;
};

//  requires c1 and c2 to be sorted
template <typename Range1, typename Range2, bool AssertSort = true>
bool overlap(Range1 const& c1, Range2 const& c2) {
  if constexpr (AssertSort) {
    utl::verify(std::is_sorted(std::cbegin(c1), std::cend(c1)),
                "Must be sorted");
    utl::verify(std::is_sorted(std::cbegin(c2), std::cend(c2)),
                "Must be sorted");
  }

  size_t idx1 = 0;
  size_t idx2 = 0;

  while (idx1 != c1.size() && idx2 != c2.size()) {
    if (c1[idx1] == c2[idx2]) {
      return true;
    }

    if (c1[idx1] < c2[idx2]) {
      while (idx1 != c1.size() && c1[idx1] < c2[idx2]) ++idx1;
    } else {
      while (idx2 != c2.size() && c2[idx2] < c1[idx1]) ++idx2;
    }
  }

  return false;
}

template <typename It1, typename It2>
bool overlap_non_sorted(It1 const& c1_begin, It1 const& c1_end,
                        It2 const& c2_begin, It2 const& c2_end) {
  auto const permutation_index_1 = get_permutation_index<It1>(c1_begin, c1_end);
  auto const permutation_index_2 = get_permutation_index<It2>(c2_begin, c2_end);

  return overlap<sorted_wrapper<It1>, sorted_wrapper<It2>, false>(
      sorted_wrapper<It1>(c1_begin, permutation_index_1),
      sorted_wrapper<It2>(c2_begin, permutation_index_2));
}

template <typename Container1, typename Container2>
  requires is_random_access_iterable<Container1> &&
           is_random_access_iterable<Container2> bool
overlap_non_sorted(Container1 const& c1, Container2 const& c2) {
  return overlap_non_sorted(std::cbegin(c1), std::cend(c1), std::begin(c2),
                            std::end(c2));
}

template <typename Container>
  requires is_random_access_iterable<Container>
auto get_overlap(Container const& c1, Container const& c2) {
  assert(std::is_sorted(std::cbegin(c1), std::cend(c1)));
  assert(std::is_sorted(std::cbegin(c2), std::cend(c2)));

  Container result;

  std::set_intersection(std::cbegin(c1), std::cend(c1), std::cbegin(c2),
                        std::cend(c2), std::back_inserter(result),
                        [&](auto&& a, auto&& b) { return a < b; });

  return result;
}

template <typename Container>
  requires is_random_access_iterable<Container>
auto get_overlap_non_sorted(Container const& c1, Container const& c2) {
  auto const perm_idx_1 = get_permutation_index<decltype(c1), strong_idx1>(c1);
  auto const perm_idx_2 = get_permutation_index<decltype(c2), strong_idx2>(c2);

  std::vector<strong_idx1> overlap;
  std::set_intersection(
      std::cbegin(perm_idx_1), std::cend(perm_idx_1), std::cbegin(perm_idx_2),
      std::cend(perm_idx_2), std::back_inserter(overlap),
      [&](auto const& a, auto const& b) {
        if constexpr (std::is_same_v<decltype(a), strong_idx1 const&>) {
          return c1[a.idx_] < c2[b.idx_];
        } else {
          return c2[a.idx_] < c1[b.idx_];
        }
      });

  std::sort(std::begin(overlap), std::end(overlap),
            [](auto&& a, auto&& b) { return a.idx_ < b.idx_; });

  Container result;
  result.reserve(overlap.size());
  std::transform(std::begin(overlap), std::end(overlap),
                 std::back_inserter(result),
                 [&](auto&& s_idx) { return c1[s_idx.idx_]; });

  return result;
}

}  // namespace soro::utls
