#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "utl/pipes.h"

#include "soro/utls/algo/overlap.h"

#include "soro/infrastructure/path/length.h"

namespace soro::infra {

type_set interlocking_route::valid_ends() {
  return type_set{
      {type::MAIN_SIGNAL, type::BORDER, type::BUMPER, type::TRACK_END}};
}

// TODO(julian): this could use a vec -> iterable fix
std::vector<element_ptr> interlocking_route::elements() const {
  return utl::all(nodes()) |
         utl::transform([](auto&& n) { return n->element_; }) | utl::vec();
}

bool interlocking_route::starts_on_ms() const {
  return nodes().front()->is(type::MAIN_SIGNAL);
}

bool interlocking_route::ends_on_ms() const {
  return nodes().back()->is(type::MAIN_SIGNAL);
}

node::idx interlocking_route::get_halt_idx(
    rs::FreightTrain const freight) const {
  // TODO(julian) only a single halt per signal station route for now
  assert(this->freight_halts_.size() <= 1);
  assert(this->passenger_halts_.size() <= 1);

  return static_cast<bool>(freight) ? this->freight_halts_.front()
                                    : this->passenger_halts_.front();
}

node_ptr interlocking_route::get_halt(rs::FreightTrain freight) const {
  return nodes()[get_halt_idx(freight)];
}

si::length interlocking_route::length() const {
  return get_path_length_from_elements(nodes());
}

utls::generator<element_ptr> t(interlocking_route const& ir) {
  return utls::coro_map(ir.nodes(), [](auto&& n) { return n->element_; });
}

bool interlocking_route::conflicts(ir_ptr other) const {
  if (this == other) {
    return true;
  }

  return utls::overlap_non_sorted(elements(), other->elements());
}

bool interlocking_route::follows(ir_ptr potential_previous) const {
  return starts_on_ms() && potential_previous->ends_on_ms() &&
         nodes().front()->id_ == potential_previous->nodes().back()->id_;
};

}  // namespace soro::infra