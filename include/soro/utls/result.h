#pragma once

#include <expected>
#include <system_error>

namespace soro::utls {

template <typename T>
using result = std::expected<T, std::error_code>;

template <typename T>
auto propagate(result<T> const& r) {
  utls::expect(!r, "error propagating a valid result");
  return std::unexpected(r.error());
}

}  // namespace soro::utls