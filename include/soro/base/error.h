#pragma once

#include <system_error>
#include <type_traits>

namespace soro {

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
