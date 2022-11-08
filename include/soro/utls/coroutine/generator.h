/*
 *
 *  Originally taken from:
 *    https://github.com/lewissbaker/cppcoro
 *    Copyright 2017 Lewis Baker
 *    MIT License
 *
 */

#pragma once

#if defined(__EMSCRIPTEN__)
#include <experimental/coroutine>
#else
#include <coroutine>
#endif

#include <exception>
#include <functional>
#include <iterator>
#include <type_traits>
#include <utility>

namespace soro::utls {

#if defined(__EMSCRIPTEN__)
namespace coro = std::experimental;
#else
namespace coro = std;
#endif

template <typename T>
struct generator;

template <typename T>
class generator_promise {
public:
  using value_type = std::remove_reference_t<T>;
  using reference_type = std::conditional_t<std::is_reference_v<T>, T, T&>;
  using pointer_type = value_type*;

  generator_promise() = default;

  generator<T> get_return_object() noexcept;

  // Set initial suspend to suspend_never
  // this has the following effects:
  //  - the construction process includes running the coro until the first yield
  //  - value_ is pointing at the first value already after construction
  //  - we don't have to resume() in begin(), as we already point to a valid val
  //  - therefore, we can repeatedly call begin() without changing the state,
  //    which is more in line with the behaviour of iterators
  constexpr coro::suspend_never initial_suspend() const noexcept { return {}; }
  constexpr coro::suspend_always final_suspend() const noexcept { return {}; }

  template <typename U = T,
            std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
  coro::suspend_always yield_value(std::remove_reference_t<T>& value) noexcept {
    value_ = std::addressof(value);
    return {};
  }

  coro::suspend_always yield_value(
      std::remove_reference_t<T>&& value) noexcept {
    value_ = std::addressof(value);
    return {};
  }

  void return_void() {}
  void unhandled_exception() {}

  reference_type value() const noexcept {
    return static_cast<reference_type>(*value_);
  }

  // Don't allow any use of 'co_await' inside the generator coroutine.
  template <typename U>
  coro::suspend_never await_transform(U&& value) = delete;

private:
  pointer_type value_{nullptr};
};

template <typename T, bool IsConst>
struct generator_iterator {
  using handle = coro::coroutine_handle<generator_promise<T>>;
  using const_handle = coro::coroutine_handle<generator_promise<T const>>;
  using member_handle = std::conditional_t<IsConst, const_handle, handle>;

  using iterator_category = std::input_iterator_tag;
  // What type should we use for counting elements of a potentially infinite
  // sequence?
  using difference_type = std::ptrdiff_t;

  using promise_type = std::conditional_t<IsConst, generator_promise<T const>,
                                          generator_promise<T>>;
  using value_type = typename promise_type::value_type;
  using reference = typename promise_type::reference_type;
  using pointer = typename promise_type::pointer_type;

  // Iterator needs to be default-constructible to satisfy the Range concept.
  generator_iterator() noexcept : coroutine_(nullptr) {}

  explicit generator_iterator(handle coroutine) noexcept {
    if constexpr (IsConst) {
      coroutine_ = member_handle::from_address(coroutine.address());
    } else {
      coroutine_ = coroutine;
    }
  }

  friend bool operator==(generator_iterator const& it1,
                         generator_iterator const& it2) noexcept {
    return (!it1.coroutine_ && !it2.coroutine_) ||  // end == end => true
           (!it1.coroutine_ && it2.coroutine_.done()) ||
           (!it2.coroutine_ && it1.coroutine_.done());
  }

  friend bool operator!=(generator_iterator const& it,
                         generator_iterator const& s) noexcept {
    return !(it == s);
  }

  generator_iterator& operator++() {
    coroutine_.resume();
    return *this;
  }

  // Need to provide post-increment operator to implement the 'Range' concept.
  void operator++(int) { (void)operator++(); }

  reference operator*() const noexcept { return coroutine_.promise().value(); }

  pointer operator->() const noexcept { return std::addressof(operator*()); }

private:
  member_handle coroutine_;
};

template <typename T>
struct [[nodiscard]] generator {
  using promise_type = generator_promise<T>;

  using iterator = generator_iterator<T, std::is_const_v<T>>;
  using const_iterator = generator_iterator<T, true>;

  generator() = delete;
  generator(generator&& other) noexcept : coroutine_(other.coroutine_) {
    other.coroutine_ = nullptr;
  }

  generator(const generator& other) = delete;

  ~generator() {
    if (coroutine_) {
      coroutine_.destroy();
    }
  }

  iterator begin() noexcept { return iterator{coroutine_}; }
  const_iterator begin() const noexcept { return const_iterator{coroutine_}; }

  iterator end() noexcept { return {}; }
  const_iterator end() const noexcept { return {}; }

private:
  friend class generator_promise<T>;

  explicit generator(coro::coroutine_handle<promise_type> coroutine) noexcept
      : coroutine_(coroutine) {}

  coro::coroutine_handle<promise_type> coroutine_;
};

template <typename T>
generator<T> generator_promise<T>::get_return_object() noexcept {
  using coroutine_handle = coro::coroutine_handle<generator_promise<T>>;
  return generator<T>{coroutine_handle::from_promise(*this)};
}

}  // namespace soro::utls
