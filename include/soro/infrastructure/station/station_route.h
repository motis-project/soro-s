#pragma once

#include <optional>

#include "soro/utls/container/optional.h"
#include "soro/utls/coroutine/recursive_generator.h"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/rolling_stock/freight.h"

namespace soro::infra {

struct station;
using station_ptr = soro::ptr<station>;
struct station_route_graph;

enum class course_decision : bool { STEM, BRANCH };

struct route_node {
  bool omitted() const { return omitted_; }

  node::ptr node_{nullptr};
  speed_limit::optional_ptr extra_spl_{};
  bool omitted_{false};
};

// TODO(julian) parse special overlap
struct station_route {
  using id = uint32_t;
  using ptr = soro::ptr<station_route>;

  static constexpr id INVALID = std::numeric_limits<id>::max();
  static constexpr bool valid(id const id) noexcept { return id != INVALID; }

  struct path {
    using id = uint32_t;
    using ptr = soro::ptr<path>;

    static constexpr id INVALID = std::numeric_limits<id>::max();
    static constexpr bool valid(id const id) noexcept { return id != INVALID; }

    node::idx size() const { return static_cast<node::idx>(nodes_.size()); }

    element_ptr start_{nullptr};
    element_ptr end_{nullptr};
    soro::vector<course_decision> course_{};

    soro::vector<node::ptr> nodes_{};
    soro::vector<node::idx> main_signals_{};
  };

  node::idx size() const noexcept;
  node::ptr nodes(node::idx const idx) const;
  soro::vector<node::ptr> const& nodes() const;

  bool electrified() const;

  // TODO(julian) these could be transformed into enum to make code depending
  // on these more safe
  // enum class route_type : uint8_t { THROUGH, IN, OUT, CONTAINED };
  bool is_through_route() const;
  bool is_in_route() const;
  bool is_out_route() const;
  bool is_contained_route() const;

  bool can_start_an_interlocking(station_route_graph const& srg) const;
  bool can_end_an_interlocking(station_route_graph const&) const;

  bool operator==(station_route const& o) const;
  bool operator!=(station_route const& o) const;

  utls::recursive_generator<route_node> iterate() const;
  utls::recursive_generator<route_node> from(node::idx from) const;
  utls::recursive_generator<route_node> to(node::idx to) const;
  utls::recursive_generator<route_node> from_to(node::idx from,
                                                node::idx to) const;

  node::optional_idx get_halt_idx(rs::FreightTrain freight) const;
  node::optional_ptr get_halt_node(rs::FreightTrain freight) const;

  node::optional_ptr get_runtime_checkpoint_node() const;

  id id_{INVALID};
  soro::string name_{"INVALID"};

  path::ptr path_{};
  soro::vector<node::idx> omitted_nodes_{};
  soro::vector<speed_limit> extra_speed_limits_{};
  node::optional_idx runtime_checkpoint_{};

  soro::ptr<station> station_{nullptr};
  utls::optional<station_ptr, nullptr> from_station_{};
  utls::optional<station_ptr, nullptr> to_station_{};

  si::length length_{si::INVALID<si::length>};

  node::optional_idx passenger_halt_{};
  node::optional_idx freight_halt_{};

  soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()> attributes_{
      DEFAULT_ATTRIBUTE_ARRAY};
};

}  // namespace soro::infra