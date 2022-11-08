#pragma once

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/route.h"
#include "soro/rolling_stock/freight.h"

namespace soro::infra {

struct station;

enum course_decision : bool { STEM, BRANCH };

// TODO(julian) parse special overlap
struct station_route {
  using id = uint32_t;
  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }
  using ptr = soro::ptr<station_route>;

  bool electrified() const;

  bool is_through_route() const;
  bool is_in_route() const;
  bool is_out_route() const;

  node::idx size() const noexcept { return r_.size(); }
  auto const& nodes() const noexcept { return r_.nodes_; }
  auto const& nodes(node::idx const i) const noexcept { return r_.nodes_[i]; }
  auto const& omitted_nodes() const noexcept { return r_.omitted_nodes_; }
  auto const& extra_spl() const noexcept { return r_.extra_speed_limits_; }
  auto entire(skip_omitted skip) const { return r_.entire(skip); }
  auto from_to(node::idx from, node::idx to, skip_omitted skip) const {
    return r_.from_to(from, to, skip);
  }

  node::idx get_halt_idx(rs::FreightTrain freight) const;
  node_ptr get_halt_node(rs::FreightTrain freight) const;

  route r_;

  id id_{INVALID};
  soro::string name_{"INVALID"};

  soro::ptr<station> station_{nullptr};
  soro::ptr<station> from_{nullptr};
  soro::ptr<station> to_{nullptr};

  // node_idx into nodes_ for every main signal in nodes_
  // omitted main signals are removed
  soro::vector<node::idx> main_signals_;

  si::length length_{si::INVALID<si::length>};

  node::idx passenger_halt_{node::INVALID_IDX};
  node::idx freight_halt_{node::INVALID_IDX};

  soro::vector<course_decision> course_;
  soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()> attributes_{
      DEFAULT_ATTRIBUTE_ARRAY};

  element_ptr start_element_{nullptr};
  element_ptr end_element_{nullptr};
};

}  // namespace soro::infra