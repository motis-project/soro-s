#pragma once

#include <vector>

#include "soro/utls/coroutine/generator.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/std_wrapper/find_if_reverse.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/element.h"

#include "utl/pipes/all.h"
#include "utl/pipes/iterable.h"
#include "utl/pipes/remove_if.h"

namespace soro::infra {

enum class skip : bool { No, Yes };

struct section {
  using id = uint32_t;
  using ids = soro::vector<id>;

  template <direction Dir, skip Skip = skip::Yes>
  utls::generator<element::ptr> iterate() const {
    auto const& elements =
        Dir == direction::Rising ? rising_order_ : falling_order_;

    for (auto e : elements) {
      if constexpr (Skip == skip::Yes) {
        if (e->is_directed_track_element() &&
            e->as<track_element>().rising_ != static_cast<bool>(Dir)) {
          continue;
        }
      }

      co_yield e;
    }
  }

  auto from(element::ptr const element, direction const dir) const {
    auto const get_range = [&](auto const& v) {
      auto const from = std::find(std::begin(v), std::end(v), element);
      return std::span{from, std::end(v)};
    };

    return dir == direction::Rising ? get_range(rising_order_)
                                    : get_range(falling_order_);
  }

  auto to(element::ptr const element, direction const dir) const {
    auto const get_range = [&](auto const& v) {
      auto const to = std::find(std::begin(v), std::end(v), element);
      return std::span{std::begin(v), to + 1};
    };

    return dir == direction::Rising ? get_range(rising_order_)
                                    : get_range(falling_order_);
  }

  auto from_to(element::ptr const from, element::ptr const to,
               direction const dir) const {
    auto const get_range = [&](auto const& v) {
      auto const f = std::find(std::begin(v), std::end(v), from);
      auto const t = std::find(std::begin(v), std::end(v), to);
      return std::span{f, t + 1};
    };

    return dir == direction::Rising ? get_range(rising_order_)
                                    : get_range(falling_order_);
  }

  kilometrage low_kilometrage() const {
    auto const neighbour = utls::find_if(rising_order_, [this](element::ptr e) {
      return e != first_rising() &&
             (e->is_undirected_track_element() || e->is_section_element() ||
              e->template as<track_element>().rising_);
    });

    utls::sassert(neighbour != std::end(rising_order_));

    return first_rising()->get_km(*neighbour);
  }

  kilometrage high_kilometrage() const {
    auto const neighbour =
        utls::find_if_reverse(rising_order_, [this](element::ptr e) {
          return e != last_rising() &&
                 (e->is_undirected_track_element() || e->is_section_element() ||
                  e->template as<track_element>().rising_);
        });

    utls::sassert(neighbour != std::rend(rising_order_));

    return last_rising()->get_km(*neighbour);
  }

  std::size_t size() const { return rising_order_.size(); }

  element::ptr first_rising() const { return rising_order_.front(); }
  element::ptr last_rising() const { return rising_order_.back(); }

  element::ptr first_falling() const { return falling_order_.front(); }
  element::ptr last_falling() const { return falling_order_.back(); }

  soro::vector<element::ptr> rising_order_;
  soro::vector<element::ptr> falling_order_;

  si::length length_{si::ZERO<si::length>};
  line::id line_id_{INVALID_LINE_ID};
};

}  // namespace soro::infra
