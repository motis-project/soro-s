#include "soro/infrastructure/infrastructure.h"

#include "soro/infrastructure/parsers/iss/parse_iss.h"

namespace soro::infra {

infrastructure::infrastructure(infrastructure_options const& options) {
  this->mem_ = parse_iss(options);
  this->access_ = std::addressof(std::get<infrastructure_t>(mem_));
}

infrastructure::infrastructure(infrastructure_t const* wrapped_infra) {
  this->access_ = wrapped_infra;
}

}  // namespace soro::infra
