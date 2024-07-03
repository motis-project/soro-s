#pragma once

#include <charconv>
#include <iomanip>
#include <string>

#include "utl/verify.h"

#include "soro/utls/sassert.h"

namespace soro::utls {

// replace , with .
enum replace_comma : bool { OFF, ON };

namespace detail {

constexpr bool contains_char(char const* start, char const* end,
                             char const needle) {
  return std::any_of(start, end, [&](auto&& c) { return c == needle; });
}

template <typename T>
constexpr T parse_fp(const char* const start, const char* const end) {
  T result = std::numeric_limits<T>::max();

#if defined(_LIBCPP_VERSION)
  // TODO(julian) this can be removed when libc++ supports from_chars for fp
  std::size_t processed = 0;
  result = static_cast<T>(std::stod(start, &processed));
  sassert(processed == static_cast<std::size_t>(end - start),
          "Processed only {} chars while parsing floating point {}, should "
          "have processed {}.",
          processed, start, end - start);
#else
  auto const [ptr, ec] = std::from_chars(start, end, result);

  sassert(ec == std::errc{}, "Error while parsing floating point input {}.",
          std::quoted(start));

  sassert(ptr == end, "Error while parsing floating point input {}.",
          std::quoted(start));

#endif

  return result;
}

}  // namespace detail

template <typename T, replace_comma ReplaceComma = OFF>
constexpr T parse_fp(const char* const start, std::size_t const len) {
  if constexpr (ReplaceComma) {
    char* tmp = new char[len + 1];
    memcpy(tmp, start, len);
    tmp[len] = '\0';

    std::replace(tmp, tmp + len, ',', '.');
    auto const result = detail::parse_fp<T>(tmp, tmp + len);
    delete[] tmp;

    return result;
  } else {
    sassert(!detail::contains_char(start, start + len, ','),
            "Input string {} contains a comma, but ReplaceComma is Off",
            std::quoted(start));

    return detail::parse_fp<T>(start, start + len);
  }
}

template <typename T, replace_comma ReplaceComma = OFF>
constexpr T parse_fp(const char* const start) {
  return parse_fp<T, ReplaceComma>(start, strlen(start));
}

template <typename T, replace_comma ReplaceComma = OFF>
constexpr T parse_fp(std::string_view s) {
  return parse_fp<T, ReplaceComma>(s.data(), s.size());
}

template <typename T, replace_comma ReplaceComma = OFF>
constexpr T parse_fp(std::string const& s) {
  return parse_fp<T, ReplaceComma>(s.data(), s.size());
}

template <typename T, replace_comma ReplaceComma = OFF>
constexpr T parse_fp(const char* const start, const char* const end) {
  return parse_fp<T, ReplaceComma>(
      start, static_cast<std::size_t>(std::distance(start, end)));
}

}  // namespace soro::utls