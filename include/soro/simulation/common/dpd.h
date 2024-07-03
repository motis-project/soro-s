#pragma once

#include <array>
#include <limits>
#include <type_traits>
#include <vector>

#include "utl/verify.h"

#include "soro/utls/coroutine/generator.h"
#include "soro/utls/tuple/for_each.h"

#include "soro/base/fp_precision.h"
#include "soro/base/soro_types.h"

#include "soro/simulation/common/granularity.h"

namespace soro::simulation {

using probability_t = float;

template <typename T>
constexpr T round_to_nearest_multiple(T const num, T const multiple) {
  if (multiple == static_cast<T const>(T{0}) || multiple == T{1}) {
    return num;
  }

  auto const remainder = num % multiple;
  if (remainder <= (multiple / T{2})) {
    return num - remainder;
  } else {
    return num - remainder + multiple;
  }
}

template <typename T>
struct prob_dist_iter {
  using container_t = typename T::container_t;
  using base_iterator_t = typename container_t::const_iterator;
  using primary_t = typename T::primary_t;
  using value_type = typename container_t::value_type;
  using granularity_t = typename T::granularity_t;

  explicit prob_dist_iter(T const& dist, base_iterator_t it)
      : t_{dist.first_}, it_(std::move(it)) {}

  constexpr auto get_granularity() const {
    return granularity_t{}.template get<primary_t>();
  }

  prob_dist_iter& operator++() {
    ++it_;
    t_ = t_ + get_granularity();
    return *this;
  }

  prob_dist_iter& operator--() {
    --it_;
    t_ -= get_granularity();
    return *this;
  }

  prob_dist_iter& operator+=(int off) {
    it_ += off;
    t_ += off * get_granularity();
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
prob_dist_iter(T const&,
               typename T::container_t::const_iterator) -> prob_dist_iter<T>;

template <typename Granularity, typename... Ts>
struct dpd {};

template <typename Granularity, typename T>
struct dpd<Granularity, T> {
  using granularity_t = Granularity;
  using primary_t = T;
  using inner_dpd_t = std::void_t<>;
  using container_t = std::vector<probability_t>;

  static constexpr primary_t INVALID_PRIMARY =
      std::numeric_limits<primary_t>::max();

  dpd() = default;

  auto begin() const { return prob_dist_iter{*this, std::cbegin(dpd_)}; }
  auto end() const { return prob_dist_iter{*this, std::cend(dpd_)}; }
  auto empty() const { return dpd_.empty(); }

  probability_t& operator[](primary_t const i) { return dpd_[to_idx(i)]; }

  template <typename Needle = primary_t>
  static constexpr primary_t get_granularity() noexcept {
    static_assert(std::is_same_v<Needle, primary_t>,
                  "Last level, but could not find type you were looking for.");
    return granularity_t{}.template get<primary_t>();
  }

  size_t to_idx(primary_t const i) {
    auto const granularity = get_granularity();
    auto const offset = round_to_nearest_multiple(i, granularity) - first_;
    return static_cast<size_t>(offset / granularity);
  }

  void insert(primary_t const head, probability_t const probability) {
    auto const granularity = get_granularity();
    auto const granular = round_to_nearest_multiple(head, granularity);

    if (dpd_.empty()) {
      first_ = granular;
      dpd_.emplace_back(probability);
      return;
    }

    if (granular < first_) {
      auto diff = (first_ - granular) / granularity;
      std::vector<probability_t> new_dpd(dpd_.size() + size_t{diff});
      for (soro::size_t idx = 0; idx < new_dpd.size(); ++idx) {
        if (static_cast<soro::size_t>(diff) < idx) {
          new_dpd[idx] = 0.0F;
        } else {
          new_dpd[idx + size_t{diff}] = dpd_[idx];
        }
      }
    }

    auto const idx = to_idx(head);
    if (idx >= dpd_.size()) {
      dpd_.resize(idx + 1);
    }

    dpd_[idx] += probability;
  }

  T first_{INVALID_PRIMARY};
  std::vector<probability_t> dpd_;
};

template <typename Granularity, typename T, typename... Ts>
struct dpd<Granularity, T, Ts...> {
  using granularity_t = Granularity;
  using primary_t = T;
  using inner_dpd_t = dpd<granularity_t, Ts...>;
  using container_t = std::vector<inner_dpd_t>;

  static constexpr primary_t INVALID_PRIMARY =
      std::numeric_limits<primary_t>::max();

  dpd() = default;

  auto begin() const { return prob_dist_iter{*this, std::cbegin(dpd_)}; }
  auto end() const { return prob_dist_iter{*this, std::cend(dpd_)}; }
  auto empty() const { return dpd_.empty(); }

  template <typename Needle = primary_t>
  static constexpr auto get_granularity() noexcept {
    if constexpr (std::is_same_v<Needle, primary_t>) {
      return granularity_t{}.template get<primary_t>();
    } else {
      return inner_dpd_t{}.template get_granularity<Needle>();
    }
  }

  std::size_t to_idx(primary_t const i) {
    auto const granularity = get_granularity();
    auto const offset = round_to_nearest_multiple(i, granularity) - first_;
    return static_cast<std::size_t>((offset / granularity).t_);
  }

  container_t& operator[](primary_t const i) { return dpd_[to_idx(i)]; }

  void insert(primary_t head, Ts... tail, probability_t probability) {
    auto granularity = get_granularity();
    auto const granular = round_to_nearest_multiple(head, granularity);

    if (first_ == INVALID_PRIMARY) {
      first_ = granular;
      dpd_.emplace_back();
      dpd_.back().insert(tail..., probability);
      return;
    }

    if (granular < first_) {
      auto const diff =
          static_cast<std::size_t>(((first_ - granular) / granularity).t_);
      container_t new_dpds(dpd_.size() + diff);
      for (auto idx = 0U; idx < new_dpds.size(); ++idx) {
        if (idx < diff) {
          new_dpds[idx] = {};
        } else {
          new_dpds[idx] = std::move(dpd_[idx]);
        }
      }
    }

    auto const idx = to_idx(head);
    if (idx >= dpd_.size()) {
      dpd_.resize(idx + 1);
    }

    dpd_[idx].insert(tail..., probability);
  }

  primary_t first_{INVALID_PRIMARY};
  std::vector<inner_dpd_t> dpd_;
};

template <typename Granularity, typename T>
inline probability_t subsum(dpd<Granularity, T> const& dpd) {
  probability_t p = 0.0F;
  for (auto const& [v, prob] : dpd) {
    p += prob;
  }

  return p;
}

template <typename Granularity, typename T, typename... Ts>
inline probability_t sum(dpd<Granularity, T, Ts...> const& dpd) {
  probability_t p = 0.0F;

  for (auto const& [v, dpds] : dpd) {
    p += subsum(dpds);
  }

  return p;
}

utls::generator<std::tuple<
    utls::unixtime, kilometer_per_hour,
    probability_t>> inline iterate(dpd<default_granularity, utls::unixtime,
                                       kilometer_per_hour> const& dpd) {
  for (auto p1 = 0U; p1 < dpd.dpd_.size(); ++p1) {
    auto const& m2 = dpd.dpd_[p1];
    for (auto p2 = 0U; p2 < m2.dpd_.size(); ++p2) {
      auto const& prob = m2.dpd_[p2];

      if (zero(prob)) {
        continue;
      }

      auto const time =
          dpd.first_ +
          dpd.get_granularity() * soro::utls::unixtime{static_cast<time_t>(p1)};
      auto const speed =
          m2.first_ + m2.get_granularity() + kilometer_per_hour{p2};

      co_yield std::tuple{time, speed, prob};
    }
  }
}

utls::generator<std::tuple<utls::unixtime, probability_t>> inline iterate(
    dpd<default_granularity, utls::unixtime> const& dpd) {

  for (auto p1 = 0U; p1 < dpd.dpd_.size(); ++p1) {
    auto const& prob = dpd.dpd_[p1];
    if (zero(prob)) {
      continue;
    }

    auto const time =
        dpd.first_ +
        dpd.get_granularity() * soro::utls::unixtime{static_cast<time_t>(p1)};

    co_yield std::tuple{time, prob};
  }
}

constexpr probability_t HUNDRED_PERCENT = probability_t(1.0);
constexpr probability_t ZERO_PERCENT = probability_t(0.0);

}  // namespace soro::simulation
