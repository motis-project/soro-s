#pragma once

#include <limits>
#include <span>

#include "soro/utls/container/it_range.h"

#include "soro/base/soro_types.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/line.h"

namespace soro::infra {

enum class rising : bool { NO, YES };

enum class direction : bool { Falling, Rising };

struct node;
using node_ptr = soro::ptr<node>;

struct element;
using element_id = uint32_t;
using element_ptr = soro::ptr<infra::element>;
using non_const_element_ptr = soro::non_const_ptr<infra::element>;

auto const INVALID_ELEMENT_ID = std::numeric_limits<element_id>::max();

using kilometrage = si::length;

constexpr auto const INVALID_LINE_ID = std::numeric_limits<line::id>::max();

struct end_element {
#if !(defined(SERIALIZE) || defined(__EMSCRIPTEN__))
  end_element() = default;
  end_element(end_element&&) = default;
  end_element(end_element const&) = delete;

  end_element& operator=(end_element&&) = default;
  auto operator=(end_element const&) = delete;

  ~end_element() = default;
#endif

  bool is(type const t) const noexcept { return type_ == t; }

  bool rising_{false};
  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  auto const& top() { return nodes_[0]; }
  auto const& bot() { return nodes_[1]; }

  auto const& top() const { return nodes_[0]; }
  auto const& bot() const { return nodes_[1]; }

  auto& rising_neighbour() { return neighbours_[0]; }
  auto& falling_neighbour() { return neighbours_[1]; }

  auto const& rising_neighbour() const { return neighbours_[0]; }
  auto const& falling_neighbour() const { return neighbours_[1]; }

  soro::array<node_ptr, 2> nodes_{nullptr, nullptr};
  soro::array<element_ptr, 2> neighbours_{nullptr, nullptr};

  kilometrage km_{si::INVALID<kilometrage>};
  line::id line_{INVALID_LINE_ID};
};

struct simple_element {
#if !(defined(SERIALIZE) || defined(__EMSCRIPTEN__))
  simple_element() = default;
  simple_element(simple_element&&) = default;
  simple_element(simple_element const&) = delete;

  simple_element& operator=(simple_element&&) = default;
  auto operator=(simple_element const&) = delete;

  ~simple_element() = default;
#endif

  bool is(type const t) const noexcept { return type_ == t; }

  bool rising_{false};
  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  auto& top() { return nodes_[0]; }
  auto& bot() { return nodes_[1]; }

  auto const& top() const { return nodes_[0]; }
  auto const& bot() const { return nodes_[1]; }

  auto& start_rising_neighbour() { return neighbours_[0]; }
  auto& start_falling_neighbour() { return neighbours_[1]; }
  auto& end_rising_neighbour() { return neighbours_[2]; }
  auto& end_falling_neighbour() { return neighbours_[3]; }

  auto const& start_rising_neighbour() const { return neighbours_[0]; }
  auto const& start_falling_neighbour() const { return neighbours_[1]; }
  auto const& end_rising_neighbour() const { return neighbours_[2]; }
  auto const& end_falling_neighbour() const { return neighbours_[3]; }

  soro::array<node_ptr, 2> nodes_{nullptr, nullptr};
  soro::array<element_ptr, 4> neighbours_{nullptr, nullptr, nullptr, nullptr};

  kilometrage end_km_{si::INVALID<kilometrage>};
  kilometrage start_km_{si::INVALID<kilometrage>};
  line::id end_line_{INVALID_LINE_ID};
  line::id start_line_{INVALID_LINE_ID};

  bool end_rising_{false};
};

struct track_element {
#if !(defined(SERIALIZE) || defined(__EMSCRIPTEN__))
  track_element() = default;
  track_element(track_element&&) = default;
  track_element(track_element const&) = delete;

  track_element& operator=(track_element&&) = default;
  auto operator=(track_element const&) = delete;

  ~track_element() = default;
#endif

  bool is(type const t) const noexcept { return type_ == t; }

  bool rising_{false};
  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  auto const& get_node() const { return nodes_[0]; }

  auto& behind() { return neighbours_[0]; }
  auto& ahead() { return neighbours_[1]; }

  auto const& behind() const { return neighbours_[0]; }
  auto const& ahead() const { return neighbours_[1]; }

  soro::array<node_ptr, 1> nodes_{nullptr};
  soro::array<element_ptr, 2> neighbours_{nullptr, nullptr};

  kilometrage km_{si::INVALID<kilometrage>};
  line::id line_{INVALID_LINE_ID};
};

struct undirected_track_element {
#if !(defined(SERIALIZE) || defined(__EMSCRIPTEN__))
  undirected_track_element() = default;
  undirected_track_element(undirected_track_element&&) = default;
  undirected_track_element(undirected_track_element const&) = delete;

  undirected_track_element& operator=(undirected_track_element&&) = default;
  auto operator=(undirected_track_element const&) = delete;

  ~undirected_track_element() = default;
#endif

  bool is(type const t) const noexcept { return type_ == t; }

  auto& top() { return nodes_[0]; }
  auto& bot() { return nodes_[1]; }

  auto const& top() const { return nodes_[0]; }
  auto const& bot() const { return nodes_[1]; }

  auto& start_rising_neighbour() { return neighbours_[0]; }
  auto& start_falling_neighbour() { return neighbours_[1]; }
  auto& end_rising_neighbour() { return neighbours_[2]; }
  auto& end_falling_neighbour() { return neighbours_[3]; }

  auto const& start_rising_neighbour() const { return neighbours_[0]; }
  auto const& start_falling_neighbour() const { return neighbours_[1]; }
  auto const& end_rising_neighbour() const { return neighbours_[2]; }
  auto const& end_falling_neighbour() const { return neighbours_[3]; }

  bool rising_{false};
  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  soro::array<node_ptr, 2> nodes_{nullptr, nullptr};
  soro::array<element_ptr, 4> neighbours_{nullptr, nullptr, nullptr, nullptr};

  kilometrage km_{si::INVALID<kilometrage>};
  line::id line_{INVALID_LINE_ID};
};

struct simple_switch {
#if !(defined(SERIALIZE) || defined(__EMSCRIPTEN__))
  simple_switch() = default;
  simple_switch(simple_switch&&) = default;
  simple_switch(simple_switch const&) = delete;

  simple_switch& operator=(simple_switch&&) = default;
  auto operator=(simple_switch const&) = delete;

  ~simple_switch() = default;
#endif

  bool is(type const t) const noexcept { return type_ == t; }

  bool rising_{false};
  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  auto& top() { return nodes_[0]; }
  auto& bot() { return nodes_[1]; }

  [[nodiscard]] auto const& top() const { return nodes_[0]; }
  [[nodiscard]] auto const& bot() const { return nodes_[1]; }

  auto& rising_start_neighbour() { return neighbours_[0]; }
  auto& falling_start_neighbour() { return neighbours_[1]; }
  auto& rising_stem_neighbour() { return neighbours_[2]; }
  auto& falling_stem_neighbour() { return neighbours_[3]; }
  auto& rising_branch_neighbour() { return neighbours_[4]; }
  auto& falling_branch_neighbour() { return neighbours_[5]; }

  auto const& rising_start_neighbour() const { return neighbours_[0]; }
  auto const& falling_start_neighbour() const { return neighbours_[1]; }
  auto const& rising_stem_neighbour() const { return neighbours_[2]; }
  auto const& falling_stem_neighbour() const { return neighbours_[3]; }
  auto const& rising_branch_neighbour() const { return neighbours_[4]; }
  auto const& falling_branch_neighbour() const { return neighbours_[5]; }

  soro::array<node_ptr, 2> nodes_{nullptr, nullptr};
  soro::array<element_ptr, 6> neighbours_{nullptr, nullptr, nullptr,
                                          nullptr, nullptr, nullptr};

  line::id start_line_{INVALID_LINE_ID}, stem_line_{INVALID_LINE_ID},
      branch_line_{INVALID_LINE_ID};
  kilometrage start_km_{si::INVALID<kilometrage>},
      stem_km_{si::INVALID<kilometrage>}, branch_km_{si::INVALID<kilometrage>};
  bool stem_rising_{false}, branch_rising_{false};
};

// used for crosses and crossswitches
struct cross {
#if !(defined(SERIALIZE) || defined(__EMSCRIPTEN__))
  cross() = default;
  cross(cross&&) = default;
  cross(cross const&) = delete;

  cross& operator=(cross&&) = default;
  auto operator=(cross const&) = delete;

  ~cross() = default;
#endif

  bool is(type const t) const noexcept { return type_ == t; }

  bool rising_{false};
  type type_{type::INVALID};
  element_id id_{INVALID_ELEMENT_ID};

  auto& top() { return nodes_[0]; }
  auto& bot() { return nodes_[1]; }
  auto& left() { return nodes_[2]; }
  auto& right() { return nodes_[3]; }

  auto const& top() const { return nodes_[0]; }
  auto const& bot() const { return nodes_[1]; }
  auto const& left() const { return nodes_[2]; }
  auto const& right() const { return nodes_[3]; }

  auto& rising_start_left() { return neighbours_[0]; }
  auto& falling_start_left() { return neighbours_[1]; }
  auto& rising_end_left() { return neighbours_[2]; }
  auto& falling_end_left() { return neighbours_[3]; }
  auto& rising_start_right() { return neighbours_[4]; }
  auto& falling_start_right() { return neighbours_[5]; }
  auto& rising_end_right() { return neighbours_[6]; }
  auto& falling_end_right() { return neighbours_[7]; }

  auto const& rising_start_left() const { return neighbours_[0]; }
  auto const& falling_start_left() const { return neighbours_[1]; }
  auto const& rising_end_left() const { return neighbours_[2]; }
  auto const& falling_end_left() const { return neighbours_[3]; }
  auto const& rising_start_right() const { return neighbours_[4]; }
  auto const& falling_start_right() const { return neighbours_[5]; }
  auto const& rising_end_right() const { return neighbours_[6]; }
  auto const& falling_end_right() const { return neighbours_[7]; }

  soro::array<node_ptr, 4> nodes_{nullptr, nullptr, nullptr, nullptr};
  soro::array<element_ptr, 8> neighbours_{nullptr, nullptr, nullptr, nullptr,
                                          nullptr, nullptr, nullptr, nullptr};

  bool start_left_end_right_arc_{false};
  bool start_right_end_left_arc_{false};

  line::id start_left_line_{INVALID_LINE_ID}, end_left_line_{INVALID_LINE_ID};
  line::id start_right_line_{INVALID_LINE_ID}, end_right_line_{INVALID_LINE_ID};
  kilometrage start_left_km_{si::INVALID<kilometrage>},
      end_left_km_{si::INVALID<kilometrage>};
  kilometrage start_right_km_{si::INVALID<kilometrage>},
      end_right_km_{si::INVALID<kilometrage>};
  bool end_left_rising_{false}, start_right_rising_{false},
      end_right_rising_{false};
};

struct element {
#if !(defined(SERIALIZE) || defined(__EMSCRIPTEN__))
  element() = default;
  element(element&&) = default;
  element(element const&) = delete;

  element& operator=(element&&) = default;
  auto operator=(element const&) = delete;

  ~element() = default;
#endif

  using ptr = element_ptr;

  using member_t =
      soro::variant<end_element, track_element, undirected_track_element,
                    simple_element, simple_switch, cross>;

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

  std::span<const element_ptr> neighbours() const {
    return this->e_.apply([](auto&& e) -> std::span<const element_ptr> {
      return {std::cbegin(e.neighbours_), e.neighbours_.size()};
    });
  }

  std::span<const node_ptr> nodes() const {
    return this->e_.apply([](auto&& e) -> std::span<const node_ptr> {
      return {std::cbegin(e.nodes_), e.nodes_.size()};
    });
  }

  bool is(type const t) const;

  std::string get_type_str() const;

  element_id id() const;

  bool rising() const;
  bool falling() const;

  enum type type() const;

  bool is_end_element() const;

  bool is_simple_element() const;

  bool is_track_element() const;

  bool is_section_element() const;

  bool is_undirected_track_element() const;

  bool is_directed_track_element() const;

  bool is_cross_switch() const;

  bool is_switch() const;

  kilometrage get_km(element::ptr neigh) const;
  si::length get_distance(element::ptr const neigh) const;
  node_ptr reverse_ahead(node_ptr n) const;

  member_t e_;
};

}  // namespace soro::infra
