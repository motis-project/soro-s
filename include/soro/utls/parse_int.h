#pragma once

#include <charconv>

#include "soro/utls/sassert.h"

namespace soro::utls {

template <std::integral T>
T parse_int(const char* const start, const char* const end) {
  T val = std::numeric_limits<T>::max();

  auto const [ptr, ec] = std::from_chars(start, end, val);

  utls::sassert(ec == std::errc{}, "Error while parsing integer input {}.",
                std::quoted(start));

  utls::sassert(ptr == end, "Error while parsing integer input {}.",
                std::quoted(start));

  return val;
}

template <std::integral T>
T parse_int(const char* const c) {
  return parse_int<T>(c, c + strlen(c));
}

template <std::integral T>
T parse_int(std::string const& s) {
  return parse_int<T>(s.data(), s.data() + s.size());
}

template <std::integral T>
T parse_int(std::string_view const& s) {
  return parse_int<T>(s.data(), s.data() + s.size());
}

}  // namespace soro::utls
