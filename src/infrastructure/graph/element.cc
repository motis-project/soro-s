#include "soro/infrastructure/graph/element.h"

#include <initializer_list>
#include <iterator>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/any.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/find_if.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/are_neighbours.h"
#include "soro/infrastructure/graph/detail/element_array_idx.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/line.h"

namespace soro::infra {

bool element::is(soro::infra::type const t) const { return type() == t; }

bool element::is_any(
    std::initializer_list<soro::infra::type> const types) const {
  return type() == utls::any{types};
}

element::id element::get_id() const {
  return e_.apply([](auto&& e) { return e.id_; });
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

bool element::is_undirected_track_element() const {
  return infra::is_undirected_track_element(type());
}

bool element::is_section_element() const { return !is_track_element(); }

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

soro::size_t element::direction_count() const {
  return e_.apply([](auto&& x) {
    return static_cast<soro::size_t>(
        std::decay_t<decltype(x)>::direction::invalid);
  });
}

// km(direction)
namespace detail {

template <typename Type, typename Direction>
kilometrage km(Type const t, Direction const dir) {
  return t->km_[get_km_idx(dir)];
}

}  // namespace detail

kilometrage end_element::km(direction const) const {
  return detail::km(this, direction::oneway);
}

kilometrage simple_element::km(direction const dir) const {
  return detail::km(this, dir);
}

kilometrage track_element::km(direction const dir) const {
  return detail::km(this, dir);
}

kilometrage simple_switch::km(direction const dir) const {
  return detail::km(this, dir);
}

kilometrage cross::km(direction const dir) const {
  return detail::km(this, dir);
}

kilometrage track_element::km() const {
  return detail::km(this, direction::oneway);
}

// km(element::ptr)
kilometrage end_element::km(element::ptr const) const { return km_.front(); }

kilometrage simple_element::km(element::ptr const neigh) const {
  using enum mileage_dir;
  using enum simple_element::direction;

  utls::sassert(utls::contains(neighbours_, neigh),
                "neigh {} not contained in neighbours for element {}",
                neigh->get_id(), id_);

  auto const is_first_neigh =
      neigh == neighbour(falling, first) || neigh == neighbour(rising, first);
  return is_first_neigh ? km(first) : km(second);
}

kilometrage track_element::km(element::ptr const) const { return km_.front(); }

kilometrage simple_switch::km(element::ptr const neigh) const {
  utls::sassert(utls::contains(neighbours_, neigh),
                "neigh {} not contained in neighbours for element {}",
                neigh->get_id(), id_);

  return id_ == neigh->get_id() ? kilometrage::zero()
                                : km(get_neighbour_dir(neigh));
}

kilometrage cross::km(element::ptr const neigh) const {
  utls::expect(utls::contains(neighbours_, neigh),
               "given element {} not contained in neighbours for element {}.",
               neigh->get_id(), id_);

  return id_ == neigh->get_id() ? kilometrage::zero()
                                : km(get_neighbour_dir(neigh));
}

kilometrage element::km(element::ptr const neigh) const {
  utls::expect(get_id() == neigh->get_id() || are_neighbours(this, neigh));
  return this->apply([&](auto&& x) { return x.km(neigh); });
}

// get_line
namespace detail {

template <typename Type, typename Direction>
line::id get_line(Type const& t, Direction const dir) {
  return t->lines_[get_line_idx(dir)];
}

}  // namespace detail

line::id end_element::get_line(direction const dir) const {
  return detail::get_line(this, dir);
}

line::id simple_element::get_line(direction const dir) const {
  return detail::get_line(this, dir);
}

line::id track_element::get_line(direction const dir) const {
  return detail::get_line(this, dir);
}

line::id track_element::get_line() const {
  return detail::get_line(this, direction::oneway);
}

line::id simple_switch::get_line(direction const dir) const {
  return detail::get_line(this, dir);
}

line::id cross::get_line(direction const dir) const {
  return detail::get_line(this, dir);
}

node::ptr default_reverse_ahead(node::ptr n) {
  for (auto const& in : n->reverse_edges_) {
    if (in->next_ == n) {
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
  using enum mileage_dir;
  using enum simple_switch::direction;

  auto const start_candidate_falling = n->element_->neighbour(falling, start);
  auto const start_candidate_rising = n->element_->neighbour(rising, start);

  // first try reversing into the start direction,
  // this is correct when coming from the branch and the stem
  auto it = utls::find_if(n->reverse_edges_, [&](node::ptr const in) {
    return in->next_ == n && in->element_ == utls::any{start_candidate_falling,
                                                       start_candidate_rising};
  });

  if (it != std::end(n->reverse_edges_)) return *it;

  // if we cannot reverse into the start direction
  // then we must go come from the start direction, go towards the stem
  auto const stem_candidate_falling = n->element_->neighbour(falling, stem);
  auto const stem_candidate_rising = n->element_->neighbour(rising, stem);

  it = utls::find_if(n->reverse_edges_, [&](node::ptr const in) {
    return in->next_ == n && in->element_ == utls::any{stem_candidate_falling,
                                                       stem_candidate_rising};
  });

  utls::sassert(it != std::end(n->reverse_edges_),
                "could not find reverse ahead in switch {}", e.id_);
  return *it;

  //  for (auto const& in : n->reverse_edges_) {
  //    bool const is_branch = in->element_ == e.neighbour(rising, branch) ||
  //                           in->element_ == e.neighbour(falling, branch);
  //    bool const neigh_is_switch = in->element_->is(type::SIMPLE_SWITCH);
  //
  //    if (is_branch) {
  //      continue;
  //    }
  //
  //    if (neigh_is_switch || in->next_ == n) {
  //      return in;
  //    }
  //  }
  //
  //  if (n->reverse_edges_.size() == 2 &&
  //      n->reverse_edges_.front() == n->reverse_edges_.back()) {
  //    return n->reverse_edges_.front();
  //  }

  //  throw utl::fail("Could not find reverse ahead in switch {}", e.id_);
}

node::ptr reverse_ahead(track_element const&, node::ptr n) {  // NOLINT
  return default_reverse_ahead(n);
}

node::ptr reverse_ahead(cross const&, node::ptr n) {  // NOLINT
  return default_reverse_ahead(n);
}

node::ptr element::reverse_ahead(node::ptr n) const {
  return e_.apply([&](auto&& x) { return infra::reverse_ahead(x, n); });
}

si::length element::get_distance(element::ptr const neigh) const {
  if (this == neigh) {
    return si::length::zero();
  }

  utls::sassert(utls::contains(neighbours(), neigh), "not neighboured");

  return (km(neigh) - neigh->km(this)).abs();
}

std::string element::get_type_str() const {
  return ::soro::infra::get_type_str(type());
}

// neighbours()

namespace detail {

template <typename ElementPtr>
std::span<element::ptr const> neighbours(ElementPtr const& e) {
  return {std::begin(e->neighbours_), e->neighbours_.size()};
}

}  // namespace detail

std::span<element::ptr const> end_element::neighbours() const {
  return detail::neighbours(this);
}

std::span<element::ptr const> simple_element::neighbours() const {
  return detail::neighbours(this);
}

std::span<element::ptr const> track_element::neighbours() const {
  return detail::neighbours(this);
}

std::span<element::ptr const> simple_switch::neighbours() const {
  return detail::neighbours(this);
}

std::span<element::ptr const> cross::neighbours() const {
  return detail::neighbours(this);
}

// neighbour(mileage_dir, direction)
namespace detail {

template <typename Element, typename Direction>
element::ptr neighbour(Element const& t, mileage_dir const m_dir,
                       Direction const dir) {
  auto const neighbour_idx = get_neighbour_idx(m_dir, dir);

  utls::sassert(neighbour_idx < t->neighbours_.size(),
                "neighbour idx {} not in range {}", neighbour_idx,
                t->neighbours_.size());

  return t->neighbours_[neighbour_idx];
}

}  // namespace detail

element::ptr end_element::neighbour(mileage_dir const m_dir,
                                    direction const d) const {
  return detail::neighbour(this, m_dir, d);
}

element::ptr simple_element::neighbour(mileage_dir const m_dir,
                                       direction const d) const {
  return detail::neighbour(this, m_dir, d);
}

element::ptr track_element::neighbour(mileage_dir const m_dir,
                                      direction const d) const {
  return detail::neighbour(this, m_dir, d);
}

element::ptr simple_switch::neighbour(mileage_dir const m_dir,
                                      direction const d) const {
  return detail::neighbour(this, m_dir, d);
}

element::ptr cross::neighbour(mileage_dir const m_dir,
                              direction const d) const {
  return detail::neighbour(this, m_dir, d);
}

namespace detail {

template <typename Element, typename Direction>
std::span<element::ptr const> neighbours(Element const& e,
                                         Direction const dir) {
  using enum mileage_dir;

  // make sure the falling/rising pairs are located next ot each other
  static_assert(
      detail::get_neighbour_idx(falling, cross::direction::start_left) + 1 ==
      detail::get_neighbour_idx(rising, cross::direction::start_left));

  auto const neighbour_idx = get_neighbour_idx(falling, dir);

  utls::sassert(neighbour_idx < e->neighbours_.size(),
                "neighbour idx {} out of range {}", neighbour_idx,
                e->neighbours_.size());
  utls::sassert(get_neighbour_idx(rising, dir) < e->neighbours_.size(),
                "neighbour idx {} out of range {}", neighbour_idx + 1,
                e->neighbours_.size());

  return {std::begin(e->neighbours_) + neighbour_idx, 2};
}

}  // namespace detail

std::span<element::ptr const> simple_element::neighbours(
    direction const dir) const {
  return detail::neighbours(this, dir);
}

std::span<element::ptr const> simple_switch::neighbours(
    direction const dir) const {
  return detail::neighbours(this, dir);
}

std::span<element::ptr const> cross::neighbours(direction const dir) const {
  return detail::neighbours(this, dir);
}

// node(nodes)
namespace detail {

template <typename Element, typename Nodes>
node::ptr node(Element const& e, Nodes const n) {
  auto const node_idx = get_node_idx(n);

  utls::sassert(node_idx < e->nodes_.size(), "node idx {} not in range {}",
                node_idx, e->nodes_.size());

  return e->nodes_[node_idx];
}

}  // namespace detail

node::ptr end_element::node(nodes const n) const {
  return detail::node(this, n);
}

node::ptr simple_element::node(nodes const n) const {
  return detail::node(this, n);
}

node::ptr track_element::node(nodes const n) const {
  return detail::node(this, n);
}

node::ptr track_element::node() const { return detail::node(this, nodes::one); }

node::ptr simple_switch::node(nodes const n) const {
  return detail::node(this, n);
}

node::ptr cross::node(nodes const n) const { return detail::node(this, n); }

// get_neighbour_dir(element::ptr const e)

simple_switch::direction simple_switch::get_neighbour_dir(
    element::ptr const neigh) const {

  utls::expect(are_neighbours(*this, neigh), "not neighboured");

  if (utls::contains(neighbours(direction::start), neigh)) {
    return direction::start;
  }

  if (utls::contains(neighbours(direction::stem), neigh)) {
    return direction::stem;
  }

  if (utls::contains(neighbours(direction::branch), neigh)) {
    return direction::branch;
  }

  std::unreachable();
}

cross::direction cross::get_neighbour_dir(element::ptr const e) const {
  utls::expect(are_neighbours(*this, e), "not neighboured");

  if (utls::contains(neighbours(direction::start_left), e)) {
    return direction::start_left;
  }

  if (utls::contains(neighbours(direction::end_left), e)) {
    return direction::end_left;
  }

  if (utls::contains(neighbours(direction::start_right), e)) {
    return direction::start_right;
  }

  if (utls::contains(neighbours(direction::end_right), e)) {
    return direction::end_right;
  }

  std::unreachable();
}

mileage_dir track_element::dir() const { return dir_; }

bool track_element::rising() const { return is_rising(dir_); }

std::span<node_ptr const> element::nodes() const {
  return this->e_.apply([](auto&& e) -> std::span<node_ptr const> {
    return {std::cbegin(e.nodes_), e.nodes_.size()};
  });
}

std::span<line::id const> element::lines() const {
  return this->e_.apply([](auto&& e) -> std::span<line::id const> {
    return {std::cbegin(e.lines_), e.lines_.size()};
  });
}

std::span<kilometrage const> element::kms() const {
  return this->e_.apply([](auto&& e) -> std::span<kilometrage const> {
    return {std::cbegin(e.km_), e.km_.size()};
  });
}

std::span<element::ptr> element::neighbours() {
  return this->e_.apply([](auto&& e) -> std::span<element::ptr> {
    return {std::begin(e.neighbours_), e.neighbours_.size()};
  });
}

std::span<element::ptr const> element::neighbours() const {
  return this->e_.apply(
      [](auto&& e) -> std::span<element::ptr const> { return e.neighbours(); });
}

}  // namespace soro::infra
