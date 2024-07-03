#pragma once

#include <limits>

#include "soro/utls/coroutine/recursive_generator.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/node.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

struct station;
using station_ptr = soro::ptr<station>;
struct station_route_graph;
struct infrastructure;

enum class course_decision : bool { STEM, BRANCH };

struct route_node {
  speed_limit::ptr get_speed_limit(infrastructure const& infra) const;

  bool operator==(route_node const&) const = default;

  node::ptr node_{nullptr};

  soro::small_vector<speed_limit::ptr> extra_spls_;
  speed_limit::optional_ptr alternate_spl_;
  bool omitted_{false};
};

// TODO() parse special overlap
struct station_route {
  using id = strong<uint32_t, struct _station_route_id>;
  using ptr = soro::ptr<station_route>;

  using idx = int16_t;
  using optional_idx = soro::optional<idx>;

  static constexpr id invalid() { return id::invalid(); }
  static constexpr idx invalid_idx() { return std::numeric_limits<idx>::max(); }

  struct path {
    using id = strong<uint32_t, struct _station_route_path_id>;
    using ptr = soro::ptr<path>;

    static constexpr id INVALID = std::numeric_limits<id>::max();
    static constexpr bool valid(id const id) noexcept { return id != INVALID; }

    idx size() const { return static_cast<idx>(nodes_.size()); }

    element::ptr start_;
    element::ptr end_;
    soro::vector<course_decision> course_;

    soro::vector<node::ptr> nodes_;
    soro::vector<idx> main_signals_;

    soro::vector<idx> etcs_starts_;
    soro::vector<idx> etcs_ends_;

    soro::vector<idx> lzb_starts_;
    soro::vector<idx> lzb_ends_;
  };

  struct speed_limit {
    infra::speed_limit spl_;
    idx idx_{invalid_idx()};
  };

  idx size() const noexcept;
  node::ptr nodes(idx const idx) const;
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

  bool requires_etcs(lines const& lines) const;
  bool requires_lzb(lines const& lines) const;

  bool operator==(station_route const& o) const;
  bool operator!=(station_route const& o) const;

  utls::recursive_generator<route_node> iterate() const;
  utls::recursive_generator<route_node> backwards() const;
  utls::recursive_generator<route_node> from_fwd(idx from) const;
  utls::recursive_generator<route_node> to_fwd(idx to) const;
  utls::recursive_generator<route_node> from_bwd(idx from) const;
  utls::recursive_generator<route_node> to_bwd(idx to) const;
  utls::recursive_generator<route_node> from_to(idx from, idx to) const;

  optional_idx get_halt_idx(rs::stop_mode stop_mode) const;
  node::optional_ptr get_halt_node(rs::stop_mode stop_mode) const;

  optional_idx get_runtime_checkpoint_idx() const;
  node::optional_ptr get_runtime_checkpoint_node() const;

  id id_{invalid()};
  soro::string name_{"INVALID"};

  path::ptr path_;
  soro::vector<idx> omitted_nodes_;
  soro::vector<speed_limit> alt_speed_limits_;
  soro::vector<speed_limit> extra_speed_limits_;

  soro::ptr<station> station_;
  soro::optional<station_ptr> from_station_;
  soro::optional<station_ptr> to_station_;

  si::length length_{si::length::invalid()};

  optional_idx runtime_checkpoint_;

  optional_idx passenger_halt_;
  optional_idx freight_halt_;

  soro::array<bool, STATION_ROUTE_ATTRIBUTES.size()> attributes_{
      DEFAULT_ATTRIBUTE_ARRAY};
};

}  // namespace soro::infra