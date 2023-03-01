#pragma once

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>

// TODO(julian) replace this with the c++20 <format> header when available
#include "fmt/ostream.h"

namespace soro::utls {

struct bool_with_loc {

#if defined(__clang__)
  bool_with_loc(bool const b,  // NOLINT
                const char* filename = __builtin_FILE(),
                const char* function_name = __builtin_FUNCTION(),
                unsigned line = __builtin_LINE(),
                unsigned column = __builtin_COLUMN())
#elif defined(__GNUC__)
  bool_with_loc(bool const b,  // NOLINT
                const char* filename = __builtin_FILE(),
                const char* function_name = __builtin_FUNCTION(),
                unsigned line = __builtin_LINE(), unsigned column = 0)
#else
  bool_with_loc(bool const b,  // NOLINT
                const char* filename = "", const char* function_name = "",
                unsigned line = 0, unsigned column = 0)
#endif
      : b_(b),
        filename_(filename),
        function_name_(function_name),
        line_(line),
        column_(column) {
  }

  operator bool() const noexcept { return b_; }  // NOLINT

  bool b_;
  const char* filename_;
  const char* function_name_;
  unsigned line_;
  unsigned column_;
};

#if !defined(NDEBUG)

template <typename Msg, typename... Args>
inline void sassert_impl(bool_with_loc assert_this,
                         std::string_view failure_type, Msg&& msg,
                         Args... args) {

  if (!assert_this) {
    using clock = std::chrono::system_clock;

    auto const now = clock::to_time_t(clock::now());
    struct tm tmp {};
#if _MSC_VER >= 1400
    gmtime_s(&tmp, &now);
#else
    gmtime_r(&now, &tmp);
#endif

    std::stringstream ss;

    fmt::print(ss, "[{}][{}] ", failure_type, std::put_time(&tmp, "%FT%TZ"));
    fmt::print(ss, std::forward<Msg>(msg), std::forward<Args>(args)...);
    fmt::print(ss, "\n");
    fmt::print(ss, "[FAILED HERE] {}:{}:{} in {}",
               std::filesystem::path(assert_this.filename_).filename().string(),
               assert_this.line_, assert_this.column_,
               assert_this.function_name_);
    fmt::print(ss, "\n");

    std::clog << ss.rdbuf();
    throw std::runtime_error(ss.str());
  }
}

#else

template <typename Msg, typename... Args>
inline void sassert_impl(bool const, Msg&&, Args...) {}

#endif

template <typename Msg, typename... Args>
inline void sassert(bool_with_loc assert_this, Msg&& msg, Args... args) {
  sassert_impl(assert_this, "ASSERT", msg, args...);
}

template <typename Msg, typename... Args>
inline void expects(bool_with_loc assert_this, Msg&& msg, Args... args) {
  sassert_impl(assert_this, "PRECONDITION", msg, args...);
}

template <typename Msg, typename... Args>
inline void ensure(bool_with_loc assert_this, Msg&& msg, Args... args) {
  sassert_impl(assert_this, "POSTCONDITION", msg, args...);
}

inline void ensure(bool_with_loc assert_this) {
  sassert_impl(assert_this, "POSTCONDITION", "Unknown");
}

inline void sassert(bool_with_loc const assert_this) {
  sassert_impl(assert_this, "Unknown", "I didn't specify a error message :(");
}

#if !defined(NDEBUG)
template <typename F>
inline void sasserts(F&& f) {
  f();
}

template <typename F>
inline void expects(F&& f) {
  f();
}

template <typename F>
inline void ensures(F&& f) {
  f();
}

#else
template <typename F>
inline void sasserts(F&&) {}

template <typename F>
inline void expects(F&&) {}

template <typename F>
inline void ensures(F&&) {}
#endif

}  // namespace soro::utls