#pragma once

#include <filesystem>

#include "cista/containers/variant.h"
#include "cista/mmap.h"
#include "cista/serialization.h"

#include "utl/verify.h"

namespace soro::utls {

template <typename T>
struct serializable {
  serializable() = default;

  explicit serializable(std::filesystem::path const& fp)
      : mem_{cista::buf<cista::mmap>{
            cista::mmap{fp.c_str(), cista::mmap::protection::READ}}},
        access_{cista::deserialize<T>(
            mem_.template as<cista::buf<cista::mmap>>())} {}

  void save(std::filesystem::path const& fp) const {
#if defined(SERIALIZE)
    if (cista::holds_alternative<T>(mem_)) {
      cista::buf<cista::mmap> mmap{cista::mmap{fp.c_str()}};
      cista::serialize(mmap, mem_.template as<T>());
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

  serializable(serializable const&) = delete;
  serializable& operator=(serializable const&) = delete;

  void move(serializable&& o) noexcept {
    this->mem_ = std::move(o.mem_);
    this->access_ = std::addressof(this->mem_.template as<T>());
  }

  serializable(serializable&& o) noexcept {
    this->move(std::forward<serializable>(o));
  }

  serializable& operator=(serializable&& o) noexcept {
    this->move(std::forward<serializable>(o));
    return *this;
  }

  ~serializable() {
    std::cout << "Destructing\n";
  }

  T const* operator->() const { return access_; }
  T const& operator*() const { return *access_; }

protected:
  cista::variant<cista::buf<cista::mmap>, T> mem_{T{}};
  T const* access_{std::addressof(mem_.template as<T>())};
};

}  // namespace soro::utls
