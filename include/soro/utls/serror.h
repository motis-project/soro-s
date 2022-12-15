#pragma once

#include "soro/utls/sassert.h"

namespace soro::utls {

template <typename Msg, typename... Args>
inline void serror(bool_with_loc error_when_true, Msg&& msg, Args... args) {
  error_when_true.b_ = !error_when_true.b_;
  sassert(error_when_true, msg, args...);
}

}  // namespace soro::utls
