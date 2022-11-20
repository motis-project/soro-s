#pragma once

#include "soro/utls/container/optional.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/route.h"
#include "soro/rolling_stock/freight.h"

namespace soro::infra {

struct station;
struct station_route_graph;

enum course_decision : bool { STEM, BRANCH };

// TODO(julian) parse special overlap
struct station_route {
  using id = uint32_t;
  using ptr = soro::ptr<station_route>;

  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }

  //  struct course_decision2 {
  //    node::idx idx_ : 15;
  //    bool decision_ : 1;
  //  };
  //
  //  struct course_decision_ref {
  //    node::idx idx_;
  //    uint8_t cd_idx_;
  //  };
  //
  //  struct node_ref {
  //    node::id id_ : 24;
  //    //    node::idx idx_;
  //    uint8_t next_course_decision_;
  //  };
  //
  //  node_ref freight_halt_;
  //  node_ref passenger_halt_;
  //
  //  std::vector<course_decision2> course_decisions_;
  //  node_ptr start_;
  //  node_ptr end_;

  bool electrified() const;

  bool is_through_route() const;
  bool is_in_route() const;
  bool is_out_route() const;

  bool can_start_an_interlocking(station_route_graph const& srg) const;
  bool can_end_an_interlocking(station_route_graph const&) const;

  node::idx size() const noexcept { return r_.size(); }
  auto const& nodes() const noexcept { return r_.nodes_; }
  auto const& nodes(node::idx const i) const noexcept { return r_.nodes_[i]; }
  auto const& omitted_nodes() const noexcept { return r_.omitted_nodes_; }
  auto const& extra_spl() const noexcept { return r_.extra_speed_limits_; }
  auto entire(skip_omitted skip) const { return r_.entire(skip); }
  auto from_to(node::idx from, node::idx to, skip_omitted skip) const {
    return r_.from_to(from, to, skip);
  }

  utls::optional<node::idx> get_halt_idx(rs::FreightTrain freight) const;
  utls::optional<node_ptr> get_halt_node(rs::FreightTrain freight) const;

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

  utls::optional<node::idx> passenger_halt_{};
  utls::optional<node::idx> freight_halt_{};
  //  node::idx passenger_halt_{node::INVALID_IDX};
  //  node::idx freight_halt_{node::INVALID_IDX};

  soro::vector<course_decision> course_;
  soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()> attributes_{
      DEFAULT_ATTRIBUTE_ARRAY};

  element_ptr start_element_{nullptr};
  element_ptr end_element_{nullptr};
};

}  // namespace soro::infra