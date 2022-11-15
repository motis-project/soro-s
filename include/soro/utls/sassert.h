#pragma once

#include <cassert>
#include <chrono>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iostream>

#if !defined(__EMSCRIPTEN__)
#include <source_location>
#endif

// TODO(julian) replace this with the c++20 <format> header when available
#include "fmt/ostream.h"

namespace soro::utls {

#if defined(__EMSCRIPTEN__)

template <typename Msg, typename... Args>
constexpr void sassert([[maybe_unused]] bool const assert_this,
                       [[maybe_unused]] Msg&& msg,
                       [[maybe_unused]] Args... args) {

#if !defined(NDEBUG)
  if (!assert_this) {
    using clock = std::chrono::system_clock;

    auto const now = clock::to_time_t(clock::now());
    struct tm tmp;
#if _MSC_VER >= 1400
    gmtime_s(&tmp, &now);
#else
    gmtime_r(&now, &tmp);
#endif

    fmt::print(std::clog, "[ASSERT FAIL][{}] ", std::put_time(&tmp, "%FT%TZ"));
    fmt::print(std::clog, std::forward<Msg>(msg), std::forward<Args>(args)...);
    fmt::print(std::clog, "\n");

    std::abort();
  }
#endif
}

constexpr void sassert(bool const assert_this) {
  sassert(assert_this, "I didn't specify a error message :(");
}

#else

struct bool_with_loc {
  constexpr bool_with_loc(bool const b, std::source_location const loc =
                                            std::source_location::current())
      : b_(b), loc_(loc) {}

  constexpr operator bool() const noexcept { return b_; }  // NOLINT

  bool b_;
  std::source_location loc_;
};

#if !defined(NDEBUG)
template <typename Msg, typename... Args>
constexpr void sassert(bool_with_loc const& assert_this, Msg&& msg,
                       Args... args) {

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
    fmt::print(
        ss, "[FAILED HERE] {}:{}:{} in {}",
        std::filesystem::path(assert_this.loc_.file_name()).filename().string(),
        assert_this.loc_.line(), assert_this.loc_.column(),
        assert_this.loc_.function_name());
    fmt::print(ss, "\n");

    std::clog << ss.rdbuf();

    throw std::runtime_error(ss.str());
  }
}

#else

template <typename Msg, typename... Args>
constexpr void sassert(bool_with_loc const&, Msg&&, Args...) {}

#endif

constexpr void sassert(bool_with_loc const& assert_this) {
  sassert(assert_this, "I didn't specify a error message :(");
}

#endif

}  // namespace soro::utls