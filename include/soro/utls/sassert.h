#pragma once

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <source_location>

// TODO(julian) replace this with the c++20 <format> header when available
#include "fmt/ostream.h"

namespace soro::utls {

struct bool_with_loc {
  bool_with_loc(bool const b, std::source_location const& loc =  // NOLINT
#if !defined(__clang__)
                              std::source_location::current())
#else
                              {})
#endif
      : b_(b), loc_(loc) {
  }

  operator bool() const noexcept { return b_; }  // NOLINT

  bool b_;
  std::source_location loc_;
};

#if !defined(NDEBUG)
template <typename Msg, typename... Args>
inline void sassert(bool_with_loc assert_this, Msg&& msg, Args... args) {

  if (!assert_this) {
    using clock = std::chrono::system_clock;

    auto const now = clock::to_time_t(clock::now());
    struct tm tmp;
#if _MSC_VER >= 1400
    gmtime_s(&tmp, &now);
#else
    gmtime_r(&now, &tmp);
#endif

    std::stringstream ss;

    fmt::print(ss, "[ASSERT FAIL][{}] ", std::put_time(&tmp, "%FT%TZ"));
    fmt::print(ss, std::forward<Msg>(msg), std::forward<Args>(args)...);
    fmt::print(ss, "\n");

#if !defined(__clang__)
    auto const& loc = assert_this.loc_;
    fmt::print(ss, "[FAILED HERE] {}:{}:{} in {}",
               std::filesystem::path(loc.file_name()).filename().string(),
               loc.line(), loc.column(), loc.function_name());
#else
    fmt::print(ss, "[FAILED HERE] {}:{}:{} in {}", __builtin_FILE(),
               __builtin_LINE(), __builtin_COLUMN(), __builtin_FUNCTION());
#endif
    fmt::print(ss, "\n");
    std::clog << ss.rdbuf();

    throw std::runtime_error(ss.str());
  }
}

#else

template <typename Msg, typename... Args>
inline void sassert(bool const, Msg&&, Args...) {}

#endif

inline void sassert(bool const assert_this) {
  sassert(assert_this, "I didn't specify a error message :(");
}

#if !defined(NDEBUG)
template <typename F>
inline void sasserts(F&& f) {
  f();
}
#else
template <typename F>
inline void sasserts(F&&) {}
#endif

}  // namespace soro::utls