#pragma once

#include "cista/containers/variant.h"

namespace soro::utls {

template <typename T, typename V, typename Fn>
constexpr auto execute_if(V&& v, Fn&& fn) noexcept {
  if (holds_alternative<T>(v)) {
    return fn(v.template as<T>());
  }
};

}  // namespace soro::utls
