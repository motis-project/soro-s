#pragma once

#include <concepts>
#include <iterator>
#include <vector>

#include "soro/base/soro_types.h"
#include "soro/utls/sassert.h"

namespace soro::utls {

// Container needs to provide:
// operator[]
// .size() (only for asserts)

template <typename IdContainer, typename Container>
struct id_iterator {
#if defined(SERIALIZE)
  using difference_type = std::ptrdiff_t;
#else
  using IdIt = typename IdContainer::iterator;

  static_assert(std::contiguous_iterator<typename IdContainer::iterator>);
  static_assert(std::contiguous_iterator<typename Container::iterator>);

  using iterator_concept = typename IdIt::iterator_concept;
  using iterator_category = typename IdIt::iterator_category;
  using difference_type = typename IdIt::difference_type;
#endif
  using value_type = soro::remove_pointer_t<typename Container::value_type>;
  using pointer = value_type const*;
  using reference = value_type const&;

  using container_t = Container const*;
  using base_iterator_t = IdIt;

  id_iterator() = default;
  id_iterator(base_iterator_t it, container_t c) : it_{it}, c_{c} {}

  reference deref(difference_type const idx) const {
    utls::sassert(idx >= 0);
    utls::sassert(static_cast<typename Container::size_type>(idx) < c_->size());

    if constexpr (soro::is_pointer_v<typename Container::value_type>) {
      return *((*c_)[static_cast<typename Container::size_type>(idx)]);
    } else {
      return (*c_)[static_cast<typename Container::size_type>(idx)];
    }
  }

  reference operator*() const { return deref(*it_); }

  auto operator->() const {
    if constexpr (soro::is_pointer_v<typename Container::value_type>) {
      return (*c_)[*it_];
    } else {
      return std::addressof((*c_)[*it_]);
    }
  }

  reference operator[](difference_type const idx) { return deref(it_[idx]); }

  reference operator[](difference_type const idx) const {
    return deref(it_[idx]);
  }

  id_iterator& operator--() {
    --it_;
    return *this;
  }

  id_iterator& operator++() {
    ++it_;
    return *this;
  }

  id_iterator operator++(int) {
    auto copy = *this;
    it_++;
    return copy;
  }

  id_iterator operator--(int) {
    auto copy = *this;
    it_--;
    return copy;
  }

  id_iterator& operator+=(difference_type const diff) {
    it_ += diff;
    return *this;
  }

  id_iterator& operator-=(difference_type const& diff) {
    it_ -= diff;
    return *this;
  }

  id_iterator operator+(difference_type const d) const { return it_ + d; }
  id_iterator operator-(difference_type const d) const { return it_ - d; }

  friend id_iterator operator+(difference_type const d, id_iterator const& it) {
    return id_iterator{.it_ = it.it_ + d, .c_ = it.c_};
  }

  difference_type operator-(id_iterator const& o) const { return it_ - o.it_; }

  bool operator>=(id_iterator const& o) const { return it_ >= o.it_; }
  bool operator<=(id_iterator const& o) const { return it_ <= o.it_; }
  bool operator>(id_iterator const& o) const { return it_ > o.it_; }
  bool operator<(id_iterator const& o) const { return it_ < o.it_; }
  bool operator==(id_iterator const& o) const { return it_ == o.it_; }
  bool operator!=(id_iterator const& o) const { return it_ != o.it_; }

  base_iterator_t it_;
  container_t c_;
};

template <typename T, typename Container>
using id_it = id_iterator<typename soro::vector<typename T::id>::const_iterator,
                          soro::vector<T>>;

template <typename T, typename Container>
using id_it_ptr =
    id_iterator<typename soro::vector<typename T::id>::const_iterator,
                soro::vector<typename T::ptr>>;

template <typename T, typename Container>
using id_it_ptr2 = id_iterator<typename Container::const_iterator,
                               soro::vector<typename T::ptr>>;

}  // namespace soro::utls