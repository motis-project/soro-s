#include "soro/infrastructure/graph/predicates.h"

#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

bool is_signal_eotd(element::id const element_id, infrastructure const& infra) {
  utls::sassert(infra->graph_.elements_[element_id]->is(type::EOTD),
                "asking for eotd type for element {}, no eotd", element_id);

  return infra->graph_.element_data_[element_id].as<eotd>().signal_;
}

bool is_route_eotd(element::id const element_id, infrastructure const& infra) {
  return !is_signal_eotd(element_id, infra);
}

bool is_signal_eotd(node::ptr const node, infrastructure const& infra) {
  return is_signal_eotd(node->element_->get_id(), infra);
}

bool is_route_eotd(node::ptr const node, infrastructure const& infra) {
  return is_route_eotd(node->element_->get_id(), infra);
}

bool is_halt(element::id const element_id, infrastructure const& infra) {
  return infra->graph_.elements_[element_id]->is(type::HALT);
}

bool is_ms(element::id const element_id, infrastructure const& infra) {
  return infra->graph_.elements_[element_id]->is(type::MAIN_SIGNAL);
}

}  // namespace soro::infra
