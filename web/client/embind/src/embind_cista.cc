#include "emscripten/bind.h"
#include "emscripten/fetch.h"

#include <fstream>

#include "cista/serialization.h"
#include "utl/logging.h"

#include "soro/base/soro_types.h"
#include "soro/infrastructure/infrastructure_t.h"

EMSCRIPTEN_BINDINGS(cista) {
  emscripten::class_<soro::data::generic_string>("soro::data::generic_string");
  emscripten::class_<soro::data::string,
                     emscripten::base<soro::data::generic_string>>(
      "soro::data::string")
      .function("str", &soro::data::string::str);
}
