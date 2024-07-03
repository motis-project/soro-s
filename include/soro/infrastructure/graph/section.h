#pragma once

#include <limits>

#include "soro/utls/coroutine/generator.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

enum class skip : bool { No, Yes };

struct section {
  using id = soro::strong<uint32_t, struct _section_id>;
  using ids = soro::vector<id>;

  // gives the position of an element inside a section
  enum class position : uint8_t { start, end, middle, invalid };

  static constexpr id INVALID = std::numeric_limits<id>::max();

  static constexpr id invalid() { return std::numeric_limits<id>::max(); }

  template <mileage_dir Dir, skip Skip = skip::Yes>
  utls::generator<element::ptr> iterate() const {
    auto const& elements =
        Dir == mileage_dir::rising ? rising_order_ : falling_order_;

    for (auto e : elements) {
      if constexpr (Skip == skip::Yes) {
        if (e->is_track_element() && e->as<track_element>().dir_ != Dir) {
          continue;
        }
      }

      co_yield e;
    }
  }

  std::span<element::ptr const> from(element::ptr const element,
                                     mileage_dir const dir) const;

  std::span<element::ptr const> to(element::ptr const element,
                                   mileage_dir const dir) const;

  std::span<element::ptr const> from_to(element::ptr const from,
                                        element::ptr const to,
                                        mileage_dir const dir) const;

  si::length length() const;

  element::ptr first_rising() const;
  element::ptr last_rising() const;

  element::ptr second_rising() const;
  element::ptr second_to_last_rising() const;

  element::ptr first_falling() const;
  element::ptr last_falling() const;

  element::ptr opposite_end(element::ptr const from) const;

  soro::size_t size() const;

  bool is_empty_border_section() const;

  bool ok() const;

  soro::vector<element::ptr> rising_order_;
  soro::vector<element::ptr> falling_order_;
};

constexpr bool is_start(section::position const pos) {
  return pos == section::position::start;
}

constexpr bool is_end(section::position const pos) {
  return pos == section::position::end;
}

constexpr bool is_boundary(section::position const pos) {
  return is_start(pos) || is_end(pos);
}

inline section::position opposite(section::position const pos) {
  utls::expect(is_boundary(pos), "no opposite for middle");
  return is_start(pos) ? section::position::end : section::position::start;
}

struct sections {
  using sections_t = soro::vector_map<section::id, section>;
  using it = sections_t::const_iterator;

  section& operator[](section::id const id);
  section const& operator[](section::id const id) const;

  soro::size_t size() const;

  it begin() const;
  it end() const;

  section::id create_section();

  bool ok() const;

  void add_section_id_to_element(element const& e, std::string const& node_name,
                                 section::position const pos,
                                 section::id const section_id);

  template <typename Direction>
  section::position get_section_position(element::id const e_id,
                                         Direction const dir) const {
    auto const& section = get_section(e_id, dir);
    if (section.first_rising()->get_id() == e_id) {
      return section::position::start;
    }

    if (section.last_rising()->get_id() == e_id) {
      return section::position::end;
    }

    utls::sassert(element_to_section_ids_[e_id].size() == 1,
                  "must be track element");

    return section::position::middle;
  }

  template <typename Direction>
  section::id get_section_id(element::id const id, Direction const dir) const {
    auto const idx = static_cast<std::underlying_type_t<Direction>>(dir);
    utls::sassert(idx < element_to_section_ids_[id].size(), "idx not in range");
    return element_to_section_ids_[id][idx];
  }

  template <typename Direction>
  section const& get_section(element::id const id, Direction const dir) const {
    return sections_[get_section_id(id, dir)];
  }

  sections_t sections_;
  soro::vecvec<element::id, section::id> element_to_section_ids_;
};

// debug purposes only ...
// otherwise the gdb cannot construct a span for pretty printing a vecvec
// TODO(julian) find a different solution to the problem
inline std::span<section::id> get_my_span(section::id* data, std::size_t size) {
  return {data, size};
}

}  // namespace soro::infra
