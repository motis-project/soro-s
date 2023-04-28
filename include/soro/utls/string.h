#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "soro/base/soro_types.h"

namespace soro::utls {

using hash = uint32_t;

// FNV-1a 32bit hashing algorithm
constexpr hash str_hash(char const* c, std::size_t const size) {
  return ((static_cast<bool>(size) ? str_hash(c, size - 1) : 2166136261U) ^
          static_cast<uint8_t>(c[size])) *
         16777619U;
}

constexpr hash str_hash(char const* c) {
  size_t size = 0;
  while (c[size] != '\0') {
    ++size;
  }
  return str_hash(c, size);
}

inline hash str_hash(std::string const& str) {
  return str_hash(str.data(), str.length());
}

inline bool equal(const char* const c1, const char* const c2) {
  return strcmp(c1, c2) == 0;
}

inline bool equal(soro::string const& s1, char const* const s2) {
  return equal(s1.data(), s2);
}

inline bool equal(char const* const s1, soro::string const& s2) {
  return equal(s1, s2.data());
}

inline bool equal(soro::string const& s1, soro::string const& s2) {
  return equal(s1.data(), s2.data());
}

inline bool equal(soro::string_view s1, char const* const s2) {
  return equal(s1.data(), s2);
}

inline bool equal(char const* const s1, soro::string_view s2) {
  return equal(s1, s2.data());
}

inline bool equal(soro::string_view const& s1, soro::string_view const& s2) {
  return equal(s1.data(), s2.data());
}

inline bool starts_with(char const* const str, char const* const prfx) {
  return strncmp(prfx, str, strlen(prfx)) == 0;
}

// inline auto split(std::string const& string, std::string const& delim) {
//   std::vector<std::string> tokens;
//
//   std::size_t start = 0;
//   std::size_t end = string.find(delim);
//   while (end != std::string::npos) {
//     tokens.push_back(string.substr(start, end - start));
//     start = end + delim.size();
//     end = string.find(delim, start);
//   }
//
//   tokens.push_back(string.substr(start, end));
//   return tokens;
// }

constexpr std::vector<std::string_view> split(std::string_view const string,
                                              std::string_view const delim) {
  if (string.empty()) {
    return {};
  }

  std::vector<std::string_view> tokens;

  std::size_t start = 0;
  std::size_t end = string.find(delim);
  while (end != std::string::npos) {
    tokens.push_back(string.substr(start, end - start));
    start = end + delim.size();
    end = string.find(delim, start);
  }

  tokens.push_back(string.substr(start, end));
  return tokens;
}

}  // namespace soro::utls