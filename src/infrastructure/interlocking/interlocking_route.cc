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

station_route::id interlocking_route::first_sr_id() const {
  return this->station_routes_.front();
}

station_route::id interlocking_route::sr_id(sr_offset const sr_offset) const {
  return this->station_routes_[sr_offset];
}

station_route::id interlocking_route::last_sr_id() const {
  return this->station_routes_.back();
}

station_route::ptr interlocking_route::first_sr(
    infrastructure const& infra) const {
  return infra->station_routes_[this->first_sr_id()];
}

station_route::ptr interlocking_route::sr(sr_offset const sr_offset,
                                          infrastructure const& infra) const {
  return infra->station_routes_[this->sr_id(sr_offset)];
}

station_route::ptr interlocking_route::last_sr(
    infrastructure const& infra) const {
  return infra->station_routes_[this->last_sr_id()];
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

bool interlocking_route::follows(interlocking_route const& other,
                                 infrastructure const& infra) const {
  return this->first_node(infra) == other.last_node(infra);
}

bool interlocking_route::operator==(interlocking_route const& o) const {
  return this->id_ == o.id_;
}

utls::recursive_generator<node::id const> interlocking_route::first_sr_nodes(
    skip_omitted const skip, base_infrastructure const& infra) const {
  auto const first_sr = infra.station_routes_[this->first_sr_id()];
  for (auto rn : first_sr->from(this->start_offset_, skip)) {
    co_yield rn.node_->id_;
  }
}

utls::recursive_generator<node::id const> interlocking_route::last_sr_nodes(
    skip_omitted const skip, base_infrastructure const& infra) const {
  auto const last_sr = infra.station_routes_[this->last_sr_id()];
  for (auto rn : last_sr->to(this->end_offset_, skip)) {
    co_yield rn.node_->id_;
  }
}

// utls::recursive_generator<route_node> interlocking_route::from_to(
//     node::idx const from, node::idx const to, skip_omitted,
//     infrastructure const&) const {}
// utls::recursive_generator<route_node> interlocking_route::to(
//     node::idx, skip_omitted, infrastructure const&) const {}
// utls::recursive_generator<route_node> interlocking_route::from(
//     node::idx, skip_omitted, infrastructure const&) const {}

utls::recursive_generator<route_node> interlocking_route::iterate(
    skip_omitted const skip_omitted, infrastructure const& infra) const {
  co_yield first_sr(infra)->from(start_offset_, skip_omitted);

  for (auto i = 1U; i < station_routes_.size() - 1; ++i) {
    co_yield this->sr(static_cast<sr_offset>(i), infra)->iterate(skip_omitted);
  }

  co_yield last_sr(infra)->to(end_offset_, skip_omitted);
}

node::ptr interlocking_route::signal_eotd(infrastructure const& infra) const {
  for (auto const rn : this->iterate(skip_omitted::ON, infra)) {
    if (rn.node_->is(type::EOTD) &&
        infra->graph_.element_data_[rn.node_->id_].as<eotd>().signal_) {
      return rn.node_;
    }
  }

  utls::sassert(false, "No signal eotd in interlocking route {}.", id_);
  throw std::runtime_error("No signal eotd in interlocking route");
}

soro::vector<node::ptr> interlocking_route::route_eotds(
    infrastructure const& infra) const {
  std::vector<node::ptr> result;

  for (auto const rn : this->iterate(skip_omitted::ON, infra)) {
    if (rn.node_->is(type::EOTD)) {
      result.emplace_back(rn.node_);
    }
  }

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