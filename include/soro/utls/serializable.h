#pragma once

#include <filesystem>

#include "utl/overloaded.h"

#include "cista/containers/variant.h"
#include "cista/mmap.h"
#include "cista/serialization.h"

#include "utl/verify.h"

namespace soro::utls {

constexpr auto const MODE = cista::mode::NONE;
//    cista::mode::WITH_INTEGRITY | cista::mode::DEEP_CHECK;

template <typename T>
struct serializable {
  serializable() = default;

  explicit serializable(std::filesystem::path const& fp)
      : mem_{cista::mmap{fp.string().c_str(), cista::mmap::protection::READ}},
        access_{cista::deserialize<T, MODE>(mem_.template as<cista::mmap>())} {}

  ~serializable() = default;

  serializable(serializable const&) = delete;
  serializable& operator=(serializable const&) = delete;

  static bool constexpr serialization_possible() {
#if defined(SERIALIZE)
    return true;
#else
    return false;
#endif
  }

  void move(serializable&& o) noexcept {
    this->mem_ = std::move(o.mem_);

    mem_.apply(utl::overloaded{
        [this](T const& t) { access_ = std::addressof(t); },
        [this, &o](cista::mmap const&) { access_ = std::move(o.access_); }});
  }

  serializable(serializable&& o) noexcept {
    this->move(std::forward<serializable>(o));
  }

  serializable& operator=(serializable&& o) noexcept {
    this->move(std::forward<serializable>(o));
    return *this;
  }

  T const* operator->() const { return access_; }
  T const& operator*() const { return *access_; }

  void save(std::filesystem::path const& fp) const {
#if defined(SERIALIZE)
    if (cista::holds_alternative<T>(mem_)) {
      cista::buf mmap{
          cista::mmap{fp.string().c_str(), cista::mmap::protection::WRITE}};
      cista::serialize<MODE>(mmap, mem_.template as<T>());
    } else {
      throw utl::fail(
          "Saving a deserialized object to a different path not yet "
          "supported.");
    }
#else
    throw utl::fail(
        "Trying to save infrastructure to {} but compiled without "
        "serialization.",
        fp);
#endif
  }

protected:
  cista::variant<cista::mmap, T> mem_{T{}};
  T const* access_{std::addressof(mem_.template as<T>())};
};

template <typename T, typename... Args>
inline T try_deserializing(std::filesystem::path const& raw_fp,
                           Args&&... args) {
  if (T::serialization_possible() && std::filesystem::exists(raw_fp)) {
    return T(raw_fp);
  } else {
    auto t = T(std::forward<Args>(args)...);

    if (T::serialization_possible()) {
      t.save(raw_fp);
    }

    return t;
  }
}

}  // namespace soro::utls
