#pragma once

#include <map>
#include <system_error>
#include <type_traits>

#include "soro/utls/result.h"
#include "soro/utls/sassert.h"

namespace soro {

struct error_stats {
  error_stats(std::string_view const name) : name_{name} {}

  template <typename T>
  void count(utls::result<T> const& result) {
    utls::expect(!result, "error counting a valid result");
    count(result.error());
  }

  void count(std::error_code const e) { ++errors_[e]; }

  void report(std::ostream& out) const {
    out << "error report for [" << name_ << "]";
    for (auto const& [e, count] : errors_) {
      out << e.message() << ": " << count;
    }
  }

  std::string_view name_{"give me name"};
  std::map<std::error_code, std::size_t> errors_;
};

namespace error {
enum error_code_t { ok = 0, not_implemented = 1, file_not_found = 2 };
}  // namespace error

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override;
  std::string message(int ev) const noexcept override;
};

inline const std::error_category& error_category() {
  static error_category_impl instance;
  return instance;
}

namespace error {
inline std::error_code make_error_code(error_code_t e) noexcept {
  return {static_cast<int>(e), error_category()};
}
}  // namespace error

}  // namespace soro

namespace std {
template <>
struct is_error_code_enum<soro::error::error_code_t> : public std::true_type {};

}  // namespace std
