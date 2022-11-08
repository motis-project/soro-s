#pragma once

#include "emscripten/bind.h"

#include "cista/containers/vector.h"

#include "soro/base/soro_types.h"

namespace soro {

template <typename T>
auto const& custom_getter(soro::data::vector<T> const& v, size_t const idx) {
  if constexpr (cista::is_pointer_v<T> || std::is_pointer_v<T>) {
    return v[idx];
  } else {
    return *v[idx];
  }
}

template <typename T>
auto get_size(soro::data::vector<T> const& v) {
  return v.size();
}

template <typename T>
void register_cista_vector(std::string const& type_name) {
  auto const vec_name = "soro::data::vector<" + type_name + ">";
  emscripten::class_<soro::data::vector<T>>(vec_name.c_str())
      .function("get", &custom_getter<T>)
      .function("size", &get_size<T>);
}

}  // namespace soro
