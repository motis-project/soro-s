#include "soro/infrastructure/infrastructure.h"

#include "soro/infrastructure/parsers/iss/parse_iss.h"

namespace soro::infra {

infrastructure::infrastructure(infrastructure_options const& options) {
  this->mem_ = parse_iss(options);
  this->access_ = std::addressof(std::get<base_infrastructure>(mem_));
}

}  // namespace soro::infra
