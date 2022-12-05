#pragma once

#include "soro/utls/algo/slice.h"
#include "soro/utls/container/it_range.h"
#include "soro/utls/coordinates/gps.h"
#include "soro/utls/coroutine/recursive_generator.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

#include "soro/infrastructure/graph/graph.h"

namespace soro::infra {

struct border;
struct station_route;

struct station {
  using id = uint32_t;
  using ptr = soro::ptr<station>;

  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }

  using optional_ptr = utls::optional<ptr, nullptr>;

  soro::vector<station::ptr> neighbours() const;

  id id_{INVALID};

  soro::string ds100_{};

  soro::vector<section::id> sections_{};
  soro::vector<element::ptr> elements_{};

  soro::map<soro::string, soro::ptr<station_route>> station_routes_{};
  soro::map<element_id, soro::vector<soro::ptr<station_route>>>
      element_to_routes_{};

  soro::vector<border> borders_{};
};

struct border {
  using track_sign = int32_t;
  constexpr static auto const INVALID_TRACK_SIGN =
      std::numeric_limits<track_sign>::max();

  // minimum information to uniquely identify a border pair
  using id_tuple = std::tuple<station::id, station::id, line_id, track_sign>;

  auto get_id_tuple() const {
    return id_tuple{std::min(station_->id_, neighbour_->id_),
                    std::max(station_->id_, neighbour_->id_), line_,
                    track_sign_};
  }

  // only needed for the parsing process,
  // TODO(julian) candidate for removal
  soro::string neighbour_name_;

  station::ptr station_{nullptr};
  element::ptr element_{nullptr};

  station::ptr neighbour_{nullptr};
  element::ptr neighbour_element_{nullptr};

  line_id line_{INVALID_LINE_ID};
  track_sign track_sign_{INVALID_TRACK_SIGN};

  bool low_border_{false};
};

}  // namespace soro::infra