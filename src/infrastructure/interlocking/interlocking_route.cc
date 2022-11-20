#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "utl/pipes.h"

#include "soro/utls/algo/overlap.h"

#include "soro/infrastructure/path/length.h"

namespace soro::infra {

type_set interlocking_route::valid_ends() {
  return type_set{{type::MAIN_SIGNAL, type::HALT, type::BORDER, type::BUMPER,
                   type::TRACK_END}};
}

// TODO(julian): this could use a vec -> iterable fix
std::vector<element_ptr> interlocking_route::elements(
    infrastructure const& infra) const {
  std::vector<element_ptr> elements;
  for (auto const sr_id : station_routes_) {
    auto const sr = infra->station_routes_[sr_id];
    utls::append(elements, soro::to_vec(sr->nodes(),
                                        [](auto&& n) { return n->element_; }));
  }
  return elements;
}

station_route::ptr interlocking_route::first_sr(
    infrastructure const& infra) const {
  return infra->station_routes_[station_routes_.front()];
}

station_route::ptr interlocking_route::sr(sr_offset const sr_offset,
                                          infrastructure const& infra) const {
  return infra->station_routes_[this->station_routes_[sr_offset]];
}

station_route::ptr interlocking_route::last_sr(
    infrastructure const& infra) const {
  return infra->station_routes_[station_routes_.back()];
}

node::ptr interlocking_route::first_node(infrastructure const& infra) const {
  return first_sr(infra)->nodes(start_offset_);
}

node::ptr interlocking_route::last_node(infrastructure const& infra) const {
  return last_sr(infra)->nodes(end_offset_);
}

bool interlocking_route::starts_on_ms(infrastructure const& infra) const {
  return first_node(infra)->is(type::MAIN_SIGNAL);
}

bool interlocking_route::ends_on_ms(infrastructure const& infra) const {
  return last_node(infra)->is(type::MAIN_SIGNAL);
}

// node::idx interlocking_route::get_halt_idx(
//     rs::FreightTrain const freight) const {
//
//   // TODO(julian) only a single halt per signal station route for now
//   assert(this->freight_halts_.size() <= 1);
//   assert(this->passenger_halts_.size() <= 1);
//
//   return static_cast<bool>(freight) ? this->freight_halts_.front()
//                                     : this->passenger_halts_.front();
// }
//
// node_ptr interlocking_route::get_halt(rs::FreightTrain freight) const {
//   return nodes()[get_halt_idx(freight)];
// }

// si::length interlocking_route::length() const {
//   return get_path_length_from_elements(nodes());
// }

// bool interlocking_route::conflicts(ir_ptr other) const {
//   if (this == other) {
//     return true;
//   }
//
//   return utls::overlap_non_sorted(elements(), other->elements());
// }
//
bool interlocking_route::follows(interlocking_route::ptr potential_previous,
                                 infrastructure const& infra) const {
  return starts_on_ms(infra) && potential_previous->ends_on_ms(infra) &&
         first_node(infra) == potential_previous->last_node(infra);
};

utls::generator<const node::ptr> interlocking_route::iterate(
    infrastructure const& infra) const {
  auto const first_sr = infra->station_routes_[station_routes_.front()];
  for (auto i = start_offset_; i < first_sr->nodes().size() - 1; ++i) {
    co_yield first_sr->nodes(i);
  }

  for (auto i = 1U; i < station_routes_.size() - 1; ++i) {
    auto const sr = infra->station_routes_[station_routes_[i]];

    for (node::idx n_idx = 0; n_idx < sr->nodes().size() - 1; ++n_idx) {
      co_yield sr->nodes(n_idx);
    }
  }

  auto const last_sr = infra->station_routes_[station_routes_.back()];
  for (auto i = 0; i < end_offset_; ++i) {
    co_yield last_sr->nodes(i);
  }
}

node::ptr interlocking_route::signal_eotd(infrastructure const& infra) const {
  for (auto const n : this->iterate(infra)) {
    if (n->is(type::EOTD) &&
        infra->graph_.element_data_[n->id_].as<eotd>().signal_) {
      return n;
    }
  }

  utls::sassert(false, "No signal eotd in interlocking route {}.", id_);
  throw std::runtime_error("No signal eotd in interlocking route");
}

soro::vector<node::ptr> interlocking_route::route_eotds(
    infrastructure const&) const {
  std::vector<node::ptr> result;
  utls::sassert(false, "Not implemented");
  return result;
}

soro::vector<node::ptr> interlocking_route::passenger_halts(
    infrastructure const&) const {
  std::vector<node::ptr> result;
  utls::sassert(false, "Not implemented");
  return result;
}
soro::vector<node::ptr> interlocking_route::freight_halts(
    infrastructure const&) const {
  std::vector<node::ptr> result;
  utls::sassert(false, "Not implemented");
  return result;
}

}  // namespace soro::infra