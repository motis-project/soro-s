#pragma once

#include <algorithm>

#include "soro/base/soro_types.h"

namespace soro::utls {

template <typename T, typename Vector = soro::vector<T>>
struct sorted_vector {
  using value_type = T;
  using const_reference = T const&;

  sorted_vector() = default;

  sorted_vector(sorted_vector const&) = delete;
  sorted_vector& operator=(sorted_vector const&) = delete;

  sorted_vector(sorted_vector&&) noexcept = delete;
  sorted_vector& operator=(sorted_vector&&) noexcept = delete;

  ~sorted_vector() = default;

  explicit sorted_vector(Vector&& v) : vec_{std::move(v)} {
    std::sort(std::begin(vec_), std::end(vec_));
  }

  template <typename Cmp>
  sorted_vector(Vector&& v, Cmp&& cmp) : vec_{std::move(v)} {
    std::sort(std::begin(vec_), std::end(vec_), std::forward<Cmp>(cmp));
  }

  [[nodiscard]] const_reference operator[](
      std::unsigned_integral auto const idx) const noexcept {
    return vec_[idx];
  }

  [[nodiscard]] const_reference at(
      std::unsigned_integral auto const idx) const noexcept {
    return vec_.at(idx);
  }

  [[nodiscard]] auto size() const noexcept { return vec_.size(); }

  auto begin() const { return std::begin(vec_); }
  auto end() const { return std::end(vec_); }

  [[nodiscard]] Vector&& move_out() { return std::move(vec_); }

#if !defined(SERIALIZE)
private:
#endif

  Vector vec_;
};

template <typename Vector>
sorted_vector(Vector&&) -> sorted_vector<typename Vector::value_type, Vector>;

template <typename Iterable>
constexpr bool is_sorted() {
  return false;
}

template <typename Iterable>
  requires std::is_same_v<Iterable,
                          sorted_vector<typename Iterable::value_type>>
constexpr bool is_sorted() {
  return true;
}

template <typename Iterable>
concept sorted = is_sorted<Iterable>();

}  // namespace soro::utls