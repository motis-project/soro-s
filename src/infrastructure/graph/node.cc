#include "soro/infrastructure/graph/node.h"

#include <initializer_list>
#include <string>

#include "soro/utls/any.h"
#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/are_neighbours.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"

namespace soro::infra {

node::id node::get_id() const { return id_; }

bool node::is(soro::infra::type const t) const { return element_->is(t); }

bool node::is_any(std::initializer_list<soro::infra::type> const types) const {
  return type() == utls::any{types};
}

type node::type() const { return element_->type(); }

std::string node::get_type_str() const {
  return ::soro::infra::get_type_str(type());
}

node::ptr node::reverse_ahead() const {
  if (reverse_edges_.empty()) {
    return nullptr;
  }

  return (reverse_edges_.size() == 1 ? reverse_edges_.front()
                                     : element_->reverse_ahead(this));
}

kilometrage node::km(node::ptr const neighbour) const {
  utls::expect(neighbour != nullptr, "neighbour is nullptr");
  utls::expect(are_neighbours(this, neighbour) || neighbour == this,
               "not neighboured");
  return element_->km(neighbour->element_);
}

}  // namespace soro::infra