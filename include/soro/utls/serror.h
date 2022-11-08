#pragma once

#include "soro/utls/sassert.h"

namespace soro::utls {

#if defined(__EMSCRIPTEN__)

template <typename Msg, typename... Args>
constexpr void serror(bool const error_when_true, Msg&& msg, Args... args) {
  sassert(!error_when_true, msg, args...);
}

#else

template <typename Msg, typename... Args>
constexpr void serror(bool_with_loc error_when_true, Msg&& msg, Args... args) {
  error_when_true.b_ = !error_when_true.b_;
  sassert(error_when_true, msg, args...);
}

#endif

}  // namespace soro::utls
