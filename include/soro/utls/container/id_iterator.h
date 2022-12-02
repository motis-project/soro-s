#pragma once

#include <concepts>
#include <iterator>

#include "soro/utls/sassert.h"

namespace soro::utls {

template <typename IdIt, typename Container>
struct id_iterator {
  static_assert(std::contiguous_iterator<IdIt>);
  static_assert(std::contiguous_iterator<typename Container::iterator>);

  using iterator_concept = typename IdIt::iterator_concept;
  using iterator_category = typename IdIt::iterator_category;
  using difference_type = typename IdIt::difference_type;
  using value_type = typename Container::value_type;
  using pointer = value_type const*;
  using reference = value_type const&;

  using container_t = Container const*;
  using base_iterator_t = IdIt;

  id_iterator() = default;
  id_iterator(base_iterator_t it, container_t c) : it_{it}, c_{c} {}

  reference deref(difference_type const idx) const {
    utls::sassert(idx >= 0);
    utls::sassert(static_cast<typename Container::size_type>(idx) < c_->size());
    return (*c_)[static_cast<typename Container::size_type>(idx)];
  }

  reference operator*() const { return deref(*it_); }
  pointer operator->() const { return &deref(*it_); }

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

template <typename IdIt, typename Container>
id_iterator(IdIt, Container const*) -> id_iterator<IdIt, Container>;

}  // namespace soro::utls