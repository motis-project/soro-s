#pragma once

#if defined(__clang__)
#include <experimental/coroutine>
#else
#include <coroutine>
#endif

namespace soro::utls {

#if defined(__clang__)
namespace coro = std::experimental;
#else
namespace coro = std;
#endif

namespace detail {

template <typename T>
struct value_promise {
  using value_t = std::remove_reference_t<T>;
  using reference_t = std::conditional_t<std::is_reference_v<T>, T, T&>;
  using pointer_t = value_t*;

  value_promise() = default;

  coro::coroutine_handle<value_promise> get_return_object() noexcept {
    return coro::coroutine_handle<value_promise>::from_promise(*this);
  }

  coro::suspend_never initial_suspend() noexcept { return {}; }
  coro::suspend_always final_suspend() noexcept { return {}; }

  void return_void() {}
  void unhandled_exception() {}

  coro::suspend_always yield_value(reference_t v) noexcept {
    v_ = v;
    return {};
  }

  template <typename V>
  coro::suspend_always yield_value(V&& v) noexcept {
    v_ = v;
    return {};
  }

  value_t value() const { return v_; }

private:
  value_t v_{};
};

template <typename T>
struct reference_promise {
  using value_t = std::remove_reference_t<T>;
  using reference_t = T;
  using pointer_t = value_t*;

  reference_promise() = default;

  coro::coroutine_handle<reference_promise> get_return_object() noexcept {
    return {coro::coroutine_handle<reference_promise>::from_promise(*this)};
  }

  coro::suspend_never initial_suspend() noexcept { return {}; }
  coro::suspend_always final_suspend() noexcept { return {}; }

  void return_void() {}
  void unhandled_exception() {}

  coro::suspend_always yield_value(reference_t v) noexcept {
    v_ = std::addressof(v);
    return {};
  }

  template <typename V>
  coro::suspend_always yield_value(V&& v) noexcept {
    static_assert(!std::is_same_v<V, std::remove_const_t<value_t>>,
                  "No temporaries allowed!");

    v_ = std::addressof(v);
    return {};
  }

  reference_t value() const { return *v_; }

private:
  pointer_t v_{nullptr};
};

}  // namespace detail

template <typename T>
struct coro_it {
  using value_t = std::remove_reference_t<T>;
  using reference_t = value_t&;
  using pointer_t = std::add_pointer_t<value_t>;
  using deref_t =
      std::conditional_t<std::is_reference_v<T>, reference_t, value_t>;
  using promise_type =
      std::conditional_t<std::is_reference_v<T>, detail::reference_promise<T>,
                         detail::value_promise<T>>;
  using co_handle_t = coro::coroutine_handle<promise_type>;

  coro_it() = default;

  coro_it(co_handle_t co_handle) noexcept  // NO LINT
      : co_handle_{std::move(co_handle)} {}

  ~coro_it() {
    if (co_handle_) {
      co_handle_.destroy();
    }
  }

  coro_it& operator++() noexcept {
    co_handle_.resume();
    return *this;
  }

  friend bool operator==(coro_it const& it1, coro_it const& it2) noexcept {
    return (!it1.co_handle_ && it2.co_handle_.done()) ||
           (!it2.co_handle_ && it1.co_handle_.done());
  }

  friend bool operator!=(coro_it const& it1, coro_it const& it2) noexcept {
    return !(it1 == it2);
  }

  deref_t operator*() const noexcept { return co_handle_.promise().value(); }

  pointer_t operator->() const noexcept { return std::addressof(operator*()); }

private:
  co_handle_t co_handle_{nullptr};
};

}  // namespace soro::utls
