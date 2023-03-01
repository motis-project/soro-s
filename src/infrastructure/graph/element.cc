#include "soro/infrastructure/graph/element.h"

#include "utl/overloaded.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

#include "soro/infrastructure/graph/node.h"

namespace soro::infra {

bool element::is(soro::infra::type const t) const {
  return e_.apply([&](auto&& e) { return e.is(t); });
}

element_id element::id() const {
  return e_.apply([](auto&& e) { return e.id_; });
}

bool element::rising() const {
  return this->e_.apply([](auto&& e) { return e.rising_; });
}

bool element::falling() const {
  return !this->e_.apply([](auto&& e) { return e.rising_; });
}

enum type element::type() const {
  return this->e_.apply([](auto&& e) { return e.type_; });
}

bool element::is_end_element() const { return infra::is_end_element(type()); }

bool element::is_simple_element() const {
  return infra::is_simple_element(type());
}

bool element::is_track_element() const {
  return infra::is_track_element(type());
}

bool element::is_section_element() const { return !is_track_element(); }

bool element::is_directed_track_element() const {
  return infra::is_directed_track_element(type());
}

bool element::is_undirected_track_element() const {
  return infra::is_undirected_track_element(type());
}

bool element::is_cross_switch() const {
  return is(type::CROSS) && (as<cross>().start_left_end_right_arc_ ||
                             as<cross>().start_right_end_left_arc_);
}

bool element::is_switch() const {
  return is(type::SIMPLE_SWITCH) || is_cross_switch();
}

bool element::joins_tracks() const {
  return type() == type::SIMPLE_SWITCH || type() == type::CROSS;
}

kilometrage get_km(end_element const& e, element_ptr) { return e.km_; }

kilometrage get_km(simple_element const& e, element_ptr neigh) {
  utls::sassert(utls::contains(e.neighbours_, neigh),
                "Neigh {} not contained in neighbours for element {}.",
                neigh->id(), e.id_);

  if (neigh == e.start_falling_neighbour() ||
      neigh == e.start_rising_neighbour()) {
    return e.start_km_;
  } else {
    return e.end_km_;
  }
}

kilometrage get_km(simple_switch const& e, element_ptr neigh) {
  utls::sassert(utls::contains(e.neighbours_, neigh),
                "Neigh {} not contained in neighbours for element {}.",
                neigh->id(), e.id_);

  if (neigh == e.rising_start_neighbour() ||
      neigh == e.falling_start_neighbour()) {
    return e.start_km_;
  } else if (neigh == e.rising_stem_neighbour() ||
             neigh == e.falling_stem_neighbour()) {
    return e.stem_km_;
  } else {
    return e.branch_km_;
  }
}

kilometrage get_km(track_element const& e, element_ptr) { return e.km_; }

kilometrage get_km(undirected_track_element const& e, element_ptr) {
  return e.km_;
}

kilometrage get_km(cross const& e, element_ptr neigh) {
  utls::sassert(utls::contains(e.neighbours_, neigh),
                "Neigh {} not contained in neighbours for element {}.",
                neigh->id(), e.id_);

  if (neigh == e.rising_start_left() || neigh == e.falling_start_left()) {
    return e.start_left_km_;
  } else if (neigh == e.rising_end_left() || neigh == e.falling_end_left()) {
    return e.end_left_km_;
  } else if (neigh == e.rising_start_right() ||
             neigh == e.falling_start_right()) {
    return e.start_right_km_;
  } else {
    return e.end_right_km_;
  }
}

kilometrage element::get_km(element_ptr const neigh) const {
  return this->apply([&](auto&& x) { return infra::get_km(x, neigh); });
}

node::ptr default_reverse_ahead(node::ptr n) {
  for (auto const& in : n->reverse_edges_) {
    if (in->next_node_ == n) {
      return in;
    }
  }

  throw utl::fail("Could not find reverse ahead in element {}", n->id_);
}

node::ptr reverse_ahead(end_element const&, node::ptr n) {  // NOLINT
  return default_reverse_ahead(n);
}

node::ptr reverse_ahead(simple_element const&, node::ptr n) {  // NOLINT
  return default_reverse_ahead(n);
}

node::ptr reverse_ahead(simple_switch const& e, node::ptr n) {
  for (auto const& in : n->reverse_edges_) {
    bool const is_branch = in->element_ == e.rising_branch_neighbour() ||
                           in->element_ == e.falling_branch_neighbour();
    bool const neigh_is_switch = in->element_->is(type::SIMPLE_SWITCH);

    if (is_branch) {
      continue;
    }

    if (neigh_is_switch || in->next_node_ == n) {
      return in;
    }
  }

  if (n->reverse_edges_.size() == 2 &&
      n->reverse_edges_.front() == n->reverse_edges_.back()) {
    return n->reverse_edges_.front();
  }

  throw utl::fail("Could not find reverse ahead in switch {}", e.id_);
}

node::ptr reverse_ahead(track_element const&, node::ptr n) {  // NOLINT
  return default_reverse_ahead(n);
}

node::ptr reverse_ahead(undirected_track_element const&,
                        node::ptr n) {  // NOLINT
  return default_reverse_ahead(n);
}

node::ptr reverse_ahead(cross const&, node::ptr n) {  // NOLINT
  return default_reverse_ahead(n);
}

node::ptr element::reverse_ahead(node::ptr n) const {
  return e_.apply([&](auto&& x) { return infra::reverse_ahead(x, n); });
}

si::length element::get_distance(element_ptr const neigh) const {
  if (this == neigh) {
    return si::ZERO<si::length>;
  }

  assert(utls::contains(this->neighbours(), neigh));

  return abs(this->get_km(neigh) - neigh->get_km(this));
}

std::string element::get_type_str() const {
  return ::soro::infra::get_type_str(type());
}

}  // namespace soro::infra
