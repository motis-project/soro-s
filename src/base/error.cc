#include "soro/base/error.h"

namespace soro {

std::string error_category_impl::message(int ev) const noexcept {
  switch (ev) {
    case error::ok: return "access: no error";
    case error::not_implemented: return "access: not implemented";
    default: return "access: unknown error";
  }
}

const char* error_category_impl::name() const noexcept { return "soro"; }

}  // namespace soro
