#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

utls::generator<const element_ptr> section::iterate(rising const direction,
                                                    bool const skip) const {
  co_yield static_cast<bool>(direction) ? elements_.front() : elements_.back();

  if (static_cast<bool>(direction)) {
    for (auto idx = 1U; idx < elements_.size() - 1; ++idx) {
      if (skip && !elements_[idx]->is_undirected_track_element() &&
          !elements_[idx]->rising()) {
        continue;
      }

      co_yield elements_[idx];
    }
  } else {
    for (auto idx = elements_.size() - 2; idx > 0; --idx) {
      if (skip && !elements_[idx]->is_undirected_track_element() &&
          !elements_[idx]->falling()) {
        continue;
      }

      co_yield elements_[idx];
    }
  }

  co_yield static_cast<bool>(direction) ? elements_.back() : elements_.front();
}

utls::generator<const element::ptr> section::iterate(
    element::ptr const from) const {

  if (from == this->elements_.front()) {
    for (auto e : this->elements_) {
      co_yield e;
    }
  } else {
    for (ssize_t i = this->elements_.size() - 1; i > -1; --i) {
      co_yield this->elements_[i];
    }
  }
}

}  // namespace soro::infra
