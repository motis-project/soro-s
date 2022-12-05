#pragma once

#include "soro/utls/coroutine/generator.h"
#include "soro/utls/coroutine/recursive_generator.h"

namespace soro::utls {

template <typename Iterable, typename Fn>
auto coro_map(Iterable&& iterable, Fn&& fn)
    -> generator<decltype(fn(*std::begin(iterable)))> {
  for (auto&& v : iterable) {
    co_yield fn(v);
  }
}

}  // namespace soro::utls
