#pragma once

#include <vector>

#include "soro/utls/coroutine/generator.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/element.h"

#include "utl/pipes/all.h"
#include "utl/pipes/iterable.h"
#include "utl/pipes/remove_if.h"

namespace soro::infra {

enum class skip : bool { No, Yes };

struct section {
  using id = uint32_t;

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

  std::size_t size() const { return rising_order_.size(); }

  element::ptr first_rising() const { return rising_order_.front(); }
  element::ptr last_rising() const { return rising_order_.back(); }

  element::ptr first_falling() const { return falling_order_.front(); }
  element::ptr last_falling() const { return falling_order_.back(); }

  soro::vector<element::ptr> rising_order_;
  soro::vector<element::ptr> falling_order_;

  si::length length_{si::ZERO<si::length>};
  line_id line_id_{INVALID_LINE_ID};
};

}  // namespace soro::infra
