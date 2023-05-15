#pragma once

#include <charconv>

#include "soro/utls/result.h"
#include "soro/utls/sassert.h"

namespace soro::utls {

namespace detail {

template <std::integral T>
[[nodiscard]] constexpr std::tuple<std::from_chars_result, T> parse_int_impl(
    const char* const start, const char* const end) {
  T val = std::numeric_limits<T>::max();

  return {std::from_chars(start, end, val), val};
}

template <std::integral T>
[[nodiscard]] constexpr utls::result<T> try_parse_int(const char* const start,
                                                      const char* const end) {
  auto const [success, val] = parse_int_impl<T>(start, end);

  if (success.ec != std::errc{}) {
    return utls::unexpected(success.ec,
                            "error {} while parsing integer input {}",
                            success.ec, std::quoted(start));
  }

  if (success.ptr != end) {
    return utls::unexpected(std::errc::invalid_argument,
                            "error {} while parsing integer input {}",
                            std::errc::invalid_argument, std::quoted(start));
  }

  return val;
}

template <std::integral T>
[[nodiscard]] constexpr T parse_int(const char* const start,
                                    const char* const end) {
  auto const [success, val] = parse_int_impl<T>(start, end);

  utls::sassert(success.ec == std::errc{},
                "Error while parsing integer input {}.", std::quoted(start));

  utls::sassert(success.ptr == end, "Error while parsing integer input {}.",
                std::quoted(start));

  return val;
}

}  // namespace detail

template <std::integral T>
[[nodiscard]] constexpr T parse_int(const char* const start,
                                    const char* const end) {
  return detail::parse_int<T>(start, end);
}

template <std::integral T>
[[nodiscard]] constexpr T parse_int(const char* const c) {
  return detail::parse_int<T>(c, c + strlen(c));
}

template <std::integral T>
[[nodiscard]] constexpr T parse_int(std::string const& s) {
  return detail::parse_int<T>(s.data(), s.data() + s.size());
}

template <std::integral T>
[[nodiscard]] constexpr T parse_int(std::string_view const s) {
  return detail::parse_int<T>(s.data(), s.data() + s.size());
}

template <std::integral T>
[[nodiscard]] constexpr utls::result<T> try_parse_int(
    std::string_view const s) {
  return detail::try_parse_int<T>(s.data(), s.data() + s.size());
}

}  // namespace soro::utls
