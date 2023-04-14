#pragma once

#include <fmt/format.h>
#include "utl/logging.h"

#include <expected>
#include <system_error>

#include "soro/utls/sassert.h"

namespace soro::utls {

template <typename T>
using result = std::expected<T, std::error_code>;

template <typename T>
auto propagate(result<T> const& r) {
  utls::expect(!r, "error propagating a valid result");
  return std::unexpected(r.error());
}

template <typename... Args>
std::unexpected<std::error_code> unexpected(std::error_code const& e,
                                            std::string_view const message,
                                            Args&&... args) {
  uLOG(utl::err) << fmt::format(message, args...);
  return std::unexpected(e);
}

}  // namespace soro::utls