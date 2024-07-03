#pragma once

#include <algorithm>

#include "cista/serialization.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/equal.h"
#include "soro/utls/std_wrapper/fill.h"
#include "soro/utls/std_wrapper/for_each.h"

namespace soro::utls {

template <typename T, std::size_t MaxSize>
struct static_vector {
  using size_type = decltype(MaxSize);
  using iterator = T*;
  using const_iterator = T const*;

  constexpr static size_type max_size = MaxSize;

  constexpr static_vector() = default;

  constexpr explicit static_vector(size_type const size)
      : static_vector{size, T{}} {}

  constexpr static_vector(size_type const num, T&& value) : end_{num} {
    utls::expect(num <= MaxSize, "size {} not in range <= {}", num, MaxSize);
    utls::fill(*this, std::forward<T>(value));
  }

  constexpr static_vector(static_vector const& other) = default;
  constexpr static_vector& operator=(static_vector const& other) = default;

  constexpr static_vector(static_vector&& other) = default;
  constexpr static_vector& operator=(static_vector&& other) = default;

  constexpr static_vector(std::initializer_list<T> const l) {
    utls::sassert(end_ < MaxSize, "going over size {}", MaxSize);
    utls::for_each(l, [&](auto&& v) { emplace_back(v); });
    end_ = utls::narrow<size_type>(l.size());  // NOLINT
  }

  constexpr ~static_vector() {
    std::for_each(begin(), end(),
                  [](auto&& v) { std::destroy_at(std::addressof(v)); });
  }

  constexpr T& operator[](size_type const idx) { return mem_[idx]; }
  constexpr T const& operator[](size_type const idx) const { return mem_[idx]; }

  constexpr iterator begin() { return std::begin(mem_); }
  constexpr iterator end() { return begin() + end_; }

  constexpr const_iterator begin() const { return std::cbegin(mem_); }
  constexpr const_iterator end() const { return begin() + end_; }

  constexpr const_iterator cbegin() const { return std::cbegin(mem_); }
  constexpr const_iterator cend() const { return cbegin() + end_; }

  constexpr bool operator==(static_vector const& other) const {
    return utls::equal(*this, other);
  }

  constexpr size_type size() const { return end_; }

  constexpr bool empty() const { return size() == 0; }

  constexpr size_type capacity() { return MaxSize; }  // NOLINT

  constexpr void clear() {
    utls::for_each(*this, [](auto&& v) { std::destroy_at(std::addressof(v)); });
    end_ = 0;
  }

  constexpr void resize(size_type const new_size) {
    utls::expect(new_size <= MaxSize, "new size {} not in range {}", new_size,
                 MaxSize);

    if (new_size == end_) return;

    if (new_size > end_) {
      std::for_each(begin() + end_, begin() + new_size,
                    [](auto&& v) { std::construct_at(std::addressof(v)); });
    } else {  // new_size < end_
      std::for_each(begin() + new_size, begin() + end_,
                    [](auto&& v) { std::destroy_at(std::addressof(v)); });
    }
    end_ = new_size;
  }

  constexpr void erase(iterator pos) {
    utls::expect(pos <= end(), "pos iterator not in range");

    std::destroy_at(pos);
    --end_;
    std::for_each(pos, end(), [](auto&& v) { v = std::move(*(&v + 1)); });
  }

  constexpr void erase(iterator const first, iterator const last) {
    utls::expect(first <= end(), "first iterator not in range");
    utls::expect(last <= end(), "last iterator not in range");
    utls::expect(first <= last, "first iterator not before last");

    std::for_each(first, last,
                  [](auto&& v) { std::destroy_at(std::addressof(v)); });
    auto const dist = std::distance(first, last);
    std::for_each(last, end(),
                  [&dist](T& v) { *((&v) - dist) = std::move(v); });
    end_ -= utls::narrow<std::size_t>(dist);
  }

  constexpr T& front() { return mem_.front(); }
  constexpr T const& front() const { return mem_.front(); }

  constexpr T& back() { return mem_[end_]; }
  constexpr T const& back() const { return mem_[end_]; }

  template <typename... Args>
  constexpr T& emplace_back(Args... args) {
    utls::sassert(end_ != MaxSize, "going over size {}", MaxSize);
    new (&mem_[end_]) T(args...);
    ++end_;
    return back();
  }

  constexpr void push_back(T const& t) {
    utls::sassert(end_ != MaxSize, "going over size {}", MaxSize);
    mem_[end_] = t;
    ++end_;
  }

  constexpr void pop_back() {
    std::destroy_at(std::addressof(back()));
    --end_;
  }

  size_type end_{0};
  std::array<T, MaxSize> mem_;
};

template <typename T, std::size_t MaxSize>
typename static_vector<T, MaxSize>::iterator begin(
    static_vector<T, MaxSize>& v) {
  return v.begin();
}

template <typename T, std::size_t MaxSize>
typename static_vector<T, MaxSize>::iterator end(static_vector<T, MaxSize>& v) {
  return v.end();
}

#if defined(SERIALIZE)

template <typename Ctx, typename T, std::size_t MaxSize>
inline void serialize(Ctx& context, static_vector<T, MaxSize> const* v,
                      cista::offset_t const offset) {
  using cista::serialize;

  // otherwise offsetof is not working
  static_assert(std::is_standard_layout_v<static_vector<T, MaxSize>>);

  serialize(context, &v->mem_,
            offset + static_cast<cista::offset_t>(
                         offsetof(std::remove_pointer_t<decltype(v)>, mem_)));
  serialize(context, &v->end_,
            offset + static_cast<cista::offset_t>(
                         offsetof(std::remove_pointer_t<decltype(v)>, end_)));
}

template <typename Ctx, typename T, std::size_t MaxSize>
inline void deserialize(Ctx const& context, static_vector<T, MaxSize>* v) {
  using cista::deserialize;
  deserialize(context, &v->mem_);
  deserialize(context, &v->end_);
}

#endif

template <typename T, std::size_t MaxSize>
cista::hash_t type_hash(static_vector<T, MaxSize> const&, cista::hash_t h,
                        std::map<cista::hash_t, unsigned>& done) noexcept {
  h = cista::hash_combine(h, cista::hash("soro::utls::static_vector"));
  h = cista::type_hash(T{}, h, done);
  return h;
}

}  // namespace soro::utls
