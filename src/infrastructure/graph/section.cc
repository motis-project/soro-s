#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

utls::generator<const element_ptr> section::iterate(
    rising const direction) const {
  co_yield elements_.front();

  for (auto idx = 1U; idx < elements_.size() - 1; ++idx) {
    if (elements_[idx]->rising() == static_cast<bool>(direction)) {
      co_yield elements_[idx];
    }
  }

  co_yield elements_.back();
}

}  // namespace soro::infra
