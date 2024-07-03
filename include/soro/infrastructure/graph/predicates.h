#pragma once

#include "soro/utls/std_wrapper/any_of.h"

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra {

bool is_signal_eotd(element::id const element_id, infrastructure const& infra);
bool is_route_eotd(element::id const element_id, infrastructure const& infra);

bool is_signal_eotd(soro::ptr<node> const node, infrastructure const& infra);
bool is_route_eotd(soro::ptr<node> const node, infrastructure const& infra);

bool is_halt(element::id const element_id, infrastructure const& infra);

bool is_ms(element::id const element_id, infrastructure const& infra);

bool is_slope(soro::ptr<node> const node, infrastructure const& infra);
bool is_slope(element::id const element_id, infrastructure const& infra);

inline bool are_neighbours(element::ptr const e1, element::ptr const e2) {
  return utls::any_of(e1->neighbours(), [&](auto&& neigh) {
    return neigh->get_id() == e2->get_id();
  });
}

}  // namespace soro::infra