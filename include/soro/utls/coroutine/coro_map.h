#pragma once

#include "soro/utls/coroutine/generator.h"
#include "soro/utls/coroutine/recursive_generator.h"

namespace soro::utls {

template <typename Iterable, typename Fn>
auto coro_map(Iterable&& iterable, Fn&& fn) -> generator<std::invoke_result_t<
    Fn, typename std::remove_reference_t<Iterable>::iterator::reference>> {
  for (auto&& v : iterable) {
    co_yield fn(v);
  }
}

// template <typename T, typename Fn>
//// generator<std::invoke_result_t<Fn&, typename
//// generator<T>::iterator::reference>>
// auto coro_map(generator<T>&& gen, Fn&& fn)
//     -> generator<decltype(fn(*std::begin(gen)))> {
//   for (auto&& v : gen) {
//     co_yield fn(v);
//   }
// }
//
// template <typename T, typename Fn>
//// generator<std::invoke_result_t<
////     Fn&, typename recursive_generator<T>::iterator::reference>>
// auto coro_map(recursive_generator<T>&& gen, Fn&& fn)
//     -> generator<decltype(fn(*std::begin(gen)))> {
//   for (auto&& v : gen) {
//     co_yield fn(v);
//   }
// }

// template <typename FUNC, typename T>
// generator<
//     std::invoke_result_t<FUNC&, typename generator<T>::iterator::reference>>
// coro_map(generator<T> source, FUNC&& func) {
//   for (auto&& value : source) {
//     co_yield std::invoke(func, static_cast<decltype(value)>(value));
//   }
// }
//
// template <typename FUNC, typename T>
// generator<std::invoke_result_t<
//     FUNC&, typename recursive_generator<T>::iterator::reference>>
// coro_map(recursive_generator<T> source, FUNC&& func) {
//   for (auto&& value : source) {
//     co_yield std::invoke(func, static_cast<decltype(value)>(value));
//   }
// }

}  // namespace soro::utls
