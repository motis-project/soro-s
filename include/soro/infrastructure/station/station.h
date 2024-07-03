#pragma once

#include "soro/base/soro_types.h"

#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

struct border;
struct station_route;

struct station {
  using id = cista::strong<uint32_t, struct _station_id>;
  using ptr = soro::ptr<station>;

  using ds100 = soro::string;

  static constexpr id invalid() { return id::invalid(); }

  using optional_ptr = soro::optional<ptr>;

  soro::vector<station::ptr> neighbours() const;

  id id_{invalid()};

  ds100 ds100_{"INVALID_DS100"};

  soro::vector<section::id> sections_;
  soro::vector<element::ptr> elements_;

  soro::map<soro::string, soro::ptr<station_route>> station_routes_;
  soro::map<element_id, soro::vector<soro::ptr<station_route>>>
      element_to_routes_;

  soro::vector<border> borders_;
};

struct border {
  using track_sign = int32_t;
  constexpr static auto const INVALID_TRACK_SIGN =
      std::numeric_limits<track_sign>::max();

  // minimum information to uniquely identify a border pair
  using id_tuple = std::tuple<station::id, station::id, line::id, track_sign>;

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

  line::id line_{line::invalid()};
  track_sign track_sign_{INVALID_TRACK_SIGN};

  section::position pos_{section::position::middle};
};

}  // namespace soro::infra