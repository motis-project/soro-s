#include "soro/infrastructure/graph/node.h"

#include "soro/infrastructure/graph/element.h"

namespace soro::infra {

bool node::is(soro::infra::type const t) const { return element_->is(t); }
type node::type() const { return element_->type(); }

node_ptr node::reverse_ahead() const {
  if (reverse_edges_.empty()) {
    return nullptr;
  }

  return (reverse_edges_.size() == 1 ? reverse_edges_.front()
                                     : element_->reverse_ahead(this));
}

}  // namespace soro::infra