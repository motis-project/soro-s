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
  auto& operator=(serializable const&) = delete;

  serializable(serializable&&) = delete;
  auto& operator=(serializable&&) = delete;

  ~serializable() = default;

  T const* operator->() const { return access_; }
  T const& operator*() const { return *access_; }

protected:
  cista::variant<cista::buf<cista::mmap>, T> mem_{T{}};
  T* access_{std::addressof(mem_.template as<T>())};
};

}  // namespace soro::utls
