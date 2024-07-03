#pragma once

#include <limits>
#include <span>

#include "soro/base/soro_types.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/detail/element_array_idx.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/line.h"

namespace soro::infra {

struct node;
using node_ptr = soro::ptr<node>;

struct element;
// basically a fwd declaration
using element_id = soro::strong<uint32_t, struct _element_id>;
using element_ptr = soro::ptr<infra::element>;

constexpr auto INVALID_ELEMENT_ID = std::numeric_limits<element_id>::max();

struct end_element {
#if !defined(SERIALIZE)
  end_element() = default;
  end_element(end_element&&) = default;
  end_element(end_element const&) = delete;

  end_element& operator=(end_element&&) = default;
  auto operator=(end_element const&) = delete;

  ~end_element() = default;
#endif

  enum class nodes : detail::element_array_idx { top, bot };
  enum class direction : detail::element_array_idx { oneway, invalid };

  node_ptr node(nodes const n) const;

  std::span<element_ptr const> neighbours() const;
  element_ptr neighbour(mileage_dir const m_dir, direction const dir) const;

  kilometrage km(direction const dir) const;
  kilometrage km(element_ptr const neighbour) const;

  line::id get_line(direction const dir) const;

  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  soro::array<node_ptr, 2> nodes_;
  soro::array<element_ptr, 2> neighbours_;
  soro::array<kilometrage, 1> km_;
  soro::array<line::id, 1> lines_;
};

struct simple_element {
#if !(defined(SERIALIZE))
  simple_element() = default;
  simple_element(simple_element&&) = default;
  simple_element(simple_element const&) = delete;

  simple_element& operator=(simple_element&&) = default;
  auto operator=(simple_element const&) = delete;

  ~simple_element() = default;
#endif

  enum class nodes : detail::element_array_idx { top, bot };
  enum class direction : detail::element_array_idx { first, second, invalid };

  node_ptr node(nodes const n) const;

  std::span<element_ptr const> neighbours() const;
  std::span<element_ptr const> neighbours(direction const dir) const;
  element_ptr neighbour(mileage_dir const m_dir, direction const dir) const;

  line::id get_line(direction const dir) const;

  kilometrage km(direction const dir) const;
  kilometrage km(element_ptr const neighbour) const;

  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  soro::array<node_ptr, 2> nodes_;
  soro::array<element_ptr, 4> neighbours_;
  soro::array<kilometrage, 2> km_;
  soro::array<line::id, 2> lines_;
};

struct track_element {
#if !(defined(SERIALIZE))
  track_element() = default;
  track_element(track_element&&) = default;
  track_element(track_element const&) = delete;

  track_element& operator=(track_element&&) = default;
  auto operator=(track_element const&) = delete;

  ~track_element() = default;
#endif

  enum class nodes : detail::element_array_idx { one };
  enum class direction : detail::element_array_idx { oneway, invalid };

  node_ptr node() const;
  node_ptr node(nodes const n) const;

  std::span<element_ptr const> neighbours() const;
  element_ptr neighbour(mileage_dir const m_dir, direction const dir) const;

  line::id get_line() const;
  line::id get_line(direction const dir) const;

  kilometrage km() const;
  kilometrage km(direction const dir) const;
  kilometrage km(element_ptr const neighbour) const;

  mileage_dir dir() const;
  bool rising() const;

  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  soro::array<node_ptr, 1> nodes_;
  soro::array<element_ptr, 2> neighbours_;
  soro::array<kilometrage, 1> km_;
  soro::array<line::id, 1> lines_{};

  mileage_dir dir_{mileage_dir::falling};
};

struct simple_switch {
#if !(defined(SERIALIZE))
  simple_switch() = default;
  simple_switch(simple_switch&&) = default;
  simple_switch(simple_switch const&) = delete;

  simple_switch& operator=(simple_switch&&) = default;
  auto operator=(simple_switch const&) = delete;

  ~simple_switch() = default;
#endif

  enum class nodes : detail::element_array_idx { top, bot };
  enum class direction : detail::element_array_idx {
    start,
    stem,
    branch,
    invalid
  };

  node_ptr node(nodes const n) const;

  std::span<element_ptr const> neighbours() const;
  std::span<element_ptr const> neighbours(direction const dir) const;
  element_ptr neighbour(mileage_dir const m_dir, direction const dir) const;

  direction get_neighbour_dir(element_ptr const neigh) const;

  line::id get_line(direction const dir) const;

  kilometrage km(direction const dir) const;
  kilometrage km(element_ptr const neighbour) const;

  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  soro::array<node_ptr, 2> nodes_;
  soro::array<element_ptr, 6> neighbours_;
  soro::array<kilometrage, 3> km_;
  soro::array<line::id, 3> lines_{};
};

// used for crosses and crossswitches
struct cross {
#if !(defined(SERIALIZE))
  cross() = default;
  cross(cross&&) = default;
  cross(cross const&) = delete;

  cross& operator=(cross&&) = default;
  auto operator=(cross const&) = delete;

  ~cross() = default;
#endif

  enum class nodes : detail::element_array_idx {
    sl_to_el,
    el_to_sl,
    sr_to_er,
    er_to_sr
  };

  enum class direction : detail::element_array_idx {
    start_left,
    end_left,
    start_right,
    end_right,
    invalid
  };

  node_ptr node(nodes const n) const;

  std::span<element_ptr const> neighbours() const;
  std::span<element_ptr const> neighbours(direction const dir) const;
  element_ptr neighbour(mileage_dir const m_dir, direction const dir) const;

  direction get_neighbour_dir(element_ptr const e) const;

  line::id get_line(direction const dir) const;

  kilometrage km(direction const dir) const;
  kilometrage km(element_ptr const neighbour) const;

  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  soro::array<node_ptr, 4> nodes_;
  soro::array<element_ptr, 8> neighbours_;
  soro::array<kilometrage, 4> km_;
  soro::array<line::id, 4> lines_{};

  bool start_left_end_right_arc_{false};
  bool start_right_end_left_arc_{false};
};

struct element {
#if !defined(SERIALIZE)
  element() = default;
  element(element&&) = default;
  element(element const&) = delete;

  element& operator=(element&&) = default;
  auto operator=(element const&) = delete;

  ~element() = default;
#endif

  using id = element_id;
  using ptr = element_ptr;
  using ids = soro::vector<id>;

  using member_t = soro::variant<end_element, track_element, simple_element,
                                 simple_switch, cross>;

  static constexpr id invalid() { return std::numeric_limits<id>::max(); }

  template <typename T>
  T& as() {
    return this->e_.as<T>();
  }

  template <typename T>
  T const& as() const {
    return this->e_.as<T>();
  }

  template <typename Fn>
  auto apply(Fn&& f) {
    return e_.apply(std::forward<Fn>(f));
  }

  template <typename Fn>
  auto apply(Fn&& f) const {
    return e_.apply(std::forward<Fn>(f));
  }

  std::span<element_ptr> neighbours();

  std::span<element_ptr const> neighbours() const;

  std::span<node_ptr const> nodes() const;

  std::span<line::id const> lines() const;

  std::span<kilometrage const> kms() const;

  kilometrage km(element::ptr neigh) const;

  bool is(type const t) const;
  bool is_any(std::initializer_list<type> const types) const;

  std::string get_type_str() const;

  id get_id() const;

  enum type type() const;

  bool is_end_element() const;

  bool is_simple_element() const;

  bool is_track_element() const;

  bool is_undirected_track_element() const;

  bool is_section_element() const;

  bool is_cross_switch() const;

  bool is_switch() const;

  bool joins_tracks() const;

  soro::size_t direction_count() const;

  template <typename Node>
  node_ptr node(Node const n) const {
    auto const node_idx = static_cast<std::underlying_type_t<Node>>(n);

    utls::sassert(node_idx < neighbours().size(), "node idx {} not in range {}",
                  node_idx, neighbours().size());

    return nodes()[node_idx];
  }

  template <typename Direction>
  element::ptr neighbour(mileage_dir const m_dir, Direction const dir) const {
    auto const neighbour_idx = detail::get_neighbour_idx(m_dir, dir);

    utls::sassert(neighbour_idx < neighbours().size(),
                  "neighbour idx {} not in range {}", neighbour_idx,
                  neighbours().size());

    return neighbours()[neighbour_idx];
  }

  template <typename Direction>
  line::id get_line(Direction const dir) const {
    return this->e_.apply([&](auto&& e) { return e.get_line(dir); });
  }

  template <typename Direction>
  kilometrage km(Direction const dir) const {
    return this->e_.apply([&](auto&& e) { return e.km(dir); });
  }

  si::length get_distance(element::ptr const neigh) const;
  node_ptr reverse_ahead(node_ptr n) const;

  member_t e_;
};

}  // namespace soro::infra
