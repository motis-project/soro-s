#pragma once

#include "emscripten/bind.h"
#include "emscripten/fetch.h"

#include <utility>

namespace soro {

template <typename First, typename Second>
void register_pair(std::string const& name) {
  emscripten::class_<std::pair<First, Second>>(name.c_str())
      .property("first", &std::pair<First, Second>::first)
      .property("second", &std::pair<First, Second>::second);
}

}  // namespace soro
