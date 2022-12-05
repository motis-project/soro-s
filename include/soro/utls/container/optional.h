#pragma once

#include <cassert>
#include <cstddef>
#include <limits>
#include <utility>

#include "cista/serialization.h"

#include "soro/base/soro_types.h"

namespace soro::utls {

template <typename T, T INVALID>
  requires std::is_integral_v<T> || soro::is_pointer_v<T>
struct optional {
  using value_type = T;
  static const constexpr auto INVALID_VALUE = INVALID;

  constexpr optional() noexcept = default;
  constexpr optional(T const val) noexcept : val_{val} {}  // NOLINT

  constexpr optional(optional const& other) = default;
  constexpr optional& operator=(optional const&) = default;

  constexpr optional(optional&& other) noexcept = default;
  constexpr optional& operator=(optional&&) noexcept = default;

  constexpr ~optional() = default;

  constexpr optional& operator=(T const val) noexcept {
    this->val_ = val;
    return *this;
  }

  constexpr T const* operator->() const noexcept {
    assert(has_value());
    return &val_;
  }

  constexpr T* operator->() noexcept {
    assert(has_value());
    return &val_;
  }

  constexpr T const& operator*() const& noexcept { return value(); }

  constexpr T& operator*() & noexcept { return value(); }

  constexpr T const&& operator*() const&& noexcept { return value(); }

  constexpr T& operator*() && noexcept { return value(); }

  constexpr explicit operator bool() const noexcept { return has_value(); }

  constexpr bool has_value() const noexcept { return val_ != INVALID; }

  constexpr T& value() & {
    assert(has_value());
    return val_;
  }

  constexpr T const& value() const& {
    assert(has_value());
    return val_;
  }

  constexpr T& value() && {
    assert(has_value());
    return val_;
  }

  constexpr T const& value() const&& {
    assert(has_value());
    return val_;
  }

  template <typename U>
  constexpr T value_or(U&& default_value) const& noexcept {
    if (has_value()) {
      return val_;
    } else {
      return static_cast<T>(default_value);
    }
  }

  template <typename U>
  constexpr T value_or(U&& default_value) && noexcept {
    return value_or(default_value);
  }

  constexpr void reset() noexcept { val_ = INVALID_VALUE; }

  template <typename Fn>
  constexpr void execute_if(Fn&& fn) const noexcept {
    if (has_value()) {
      fn(value());
    }
  }

  template <typename ThenFunction>
  constexpr auto and_then(ThenFunction&& then_function) const noexcept {
    if (has_value()) {
      return then_function(value());
    } else {
      return std::remove_cvref_t<
          std::invoke_result_t<ThenFunction, T const&>>();
    }
  }

  template <typename TransformFunction>
  std::invoke_result_t<TransformFunction, T&> transform(
      TransformFunction&& transform_function) const noexcept {
    if (has_value()) {
      return transform_function(this->value());
    } else {
      return std::remove_cvref_t<
          std::invoke_result_t<TransformFunction, T const&>>();
    }
  }

  template <typename ElseFunction>
  constexpr optional or_else(ElseFunction&& else_function) const& noexcept {
    return has_value() ? *this : std::forward<ElseFunction>(else_function)();
  }

  template <typename ElseFunction>
  constexpr optional or_else(ElseFunction&& else_function) && noexcept {
    return or_else(else_function);
  }

  constexpr void swap(optional& other) noexcept {
    if (has_value() && !other.has_value()) {
      other = this->val_;
      this->reset();
    } else if (!has_value() && other.has_value()) {
      this->val_ = other.value();
      other.reset();
    } else if (has_value() && other.has_value()) {
      auto const tmp = this->value();
      this->val_ = other.value();
      other = tmp;
    }
  }

#if !defined(SERIALIZE)
private:
#endif
  T val_{INVALID_VALUE};
};

template <typename T, T INVALID>
  requires std::is_integral_v<T> || std::is_pointer_v<T>
optional<T, INVALID> make_optional(T&& value) {
  return optional<T, INVALID>(std::forward<T>(value));
}

template <typename Ctx, typename T, T INVALID>
inline void serialize(Ctx& context, optional<T, INVALID> const* opt,
                      cista::offset_t const offset) {
  using cista::serialize;

  // otherwise offsetof is not working
  static_assert(std::is_standard_layout_v<optional<T, INVALID>>);

  serialize(context, &opt->val_,
            offset + static_cast<cista::offset_t>(
                         offsetof(std::remove_pointer_t<decltype(opt)>, val_)));
}

template <typename Ctx, typename T, T INVALID>
inline void deserialize(Ctx const& context, optional<T, INVALID>* opt) {
  using cista::deserialize;
  deserialize(context, &opt->val_);
}

}  // namespace soro::utls