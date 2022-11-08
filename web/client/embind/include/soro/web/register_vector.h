#pragma once

#include "emscripten/bind.h"

#include "soro/web/register_cista_vector.h"

namespace soro {

template <typename T>
void register_vector(std::string const& name) {
#if defined(SERIALIZE)
  register_cista_vector<T>(name);
#else
  emscripten::register_vector<T>(name.c_str());
#endif
}

}  // namespace soro
