#pragma once

#include <limits>
#include <vector>

namespace soro {

using probability_t = float;

template <typename T>
struct prob_dist_iter {
  using container_type = typename T::container_type;
  using base_iterator_t = typename container_type::const_iterator;
  using primary_t = typename T::primary_t;
  using value_type = typename container_type::value_type;

  explicit prob_dist_iter(T const& dist, base_iterator_t it)
      : t_{dist.offset_}, it_(std::move(it)) {}

  prob_dist_iter& operator++() {
    ++it_;
    ++t_;
    return *this;
  }

  prob_dist_iter& operator--() {
    --it_;
    --t_;
    return *this;
  }

  prob_dist_iter& operator+=(int off) {
    it_ += off;
    t_ += off;
    return *this;
  }

  std::pair<primary_t, value_type> operator*() const { return {t_, *it_}; }

  bool operator==(prob_dist_iter const& o) const { return it_ == o.it_; }
  bool operator!=(prob_dist_iter const& o) const { return it_ != o.it_; }

  int operator-(prob_dist_iter const& o) const {
    return std::distance(it_, o.it_);
  }

  typename T::primary_t t_;
  base_iterator_t it_;
};

template <typename T>
prob_dist_iter(T const&, typename T::container_type::const_iterator)
    ->prob_dist_iter<T>;

template <typename Granularity, typename... Ts>
struct dpd {};

template <typename Granularity, typename T>
struct dpd<Granularity, T> {
  using primary_t = T;
  using container_type = std::vector<probability_t>;

  dpd() = default;
  explicit dpd(T t) : offset_{std::move(t)}, dpd_({probability_t{1.0}}) {}
  auto begin() const { return prob_dist_iter{*this, std::cbegin(dpd_)}; }
  auto end() const { return prob_dist_iter{*this, std::cend(dpd_)}; }
  bool empty() const { return dpd_.empty(); }

  auto get_idx(primary_t const v) const {
    return static_cast<size_t>(v - offset_);
  }

  void resize(primary_t const v) {
    if (v < offset_) {
      auto diff = static_cast<size_t>(offset_ - v);
      dpd_.resize(diff, probability_t{0});
      std::rotate(rbegin(dpd_), rbegin(dpd_) + diff, rend(dpd_));

    } else if (v > offset_ + dpd_.size()) {
      dpd_.resize(v - offset_, probability_t{0});
    }
  }

  auto const& operator[](primary_t const v) const { return dpd_[get_idx(v)]; }
  auto& operator[](primary_t const v) { return dpd_[get_idx(v)]; }

  auto set(primary_t const v, probability_t const p) {
    resize(v);
    dpd_[get_idx(v)] = p;
  }

  auto add(primary_t const v, probability_t const p) {
    resize(v);
    dpd_[get_idx(v)] += p;
  }

  static constexpr primary_t to_idx(primary_t const i) {
    constexpr auto const granularity = Granularity{}.template get<0>();
    return i / granularity;
  }

  std::vector<probability_t> dpd_;

private:
  T offset_{std::numeric_limits<T>::max()};
};

template <typename Granularity, typename T, typename... Ts>
struct dpd<Granularity, T, Ts...> {
  using primary_t = T;
  using container_type = std::vector<dpd<Granularity, Ts...>>;

  dpd() = default;

  explicit dpd(T head, Ts... tail) : offset_{std::move(head)} {
    dpd_.emplace_back(tail...);
  }
  auto begin() const { return prob_dist_iter{*this, std::cbegin(dpd_)}; }
  auto end() const { return prob_dist_iter{*this, std::cend(dpd_)}; }
  bool empty() const { return dpd_.empty(); }

  dpd<Granularity, Ts...>& operator[](primary_t const i) {
    return dpd_[to_idx(i)];
  }

  static constexpr primary_t to_idx(primary_t const i) {
    constexpr auto const granularity = Granularity{}.template get<0>();
    return i / granularity;
  }

  T offset_{std::numeric_limits<primary_t>::max()};
  std::vector<dpd<Granularity, Ts...>> dpd_;
};

}  // namespace soro
