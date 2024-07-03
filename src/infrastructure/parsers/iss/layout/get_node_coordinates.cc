#include "soro/infrastructure/parsers/iss/layout/get_node_coordinates.h"

#include <array>
#include <map>

#include "utl/enumerate.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/layout.h"

namespace soro::infra {

// offset to the top, bot, left, right
constexpr coordinates top_offset = {0.0, -0.5};
constexpr coordinates bot_offset = {0.0, 0.5};
constexpr coordinates left_offset = {-0.5, 0.0};
constexpr coordinates right_offset = {0.5, 0.0};

constexpr std::array<coordinates, 4> OFFSETS = {top_offset, bot_offset,
                                                left_offset, right_offset};

static const std::map<cross::nodes, coordinates> CROSS_OFFSETS = {
    {cross::nodes::sl_to_el, {0.25, 0.25}},
    {cross::nodes::el_to_sl, {-0.25, -0.25}},
    {cross::nodes::sr_to_er, {-0.25, 0.25}},
    {cross::nodes::er_to_sr, {0.25, -0.25}}};

soro::vector_map<node::id, coordinates> get_node_coordinates(
    soro::vector_map<element::id, coordinates> const& element_coordinates,
    infrastructure_t const& infra) {
  utl::scoped_timer const timer("creating node coordinates");

  soro::vector_map<node::id, coordinates> result(infra.graph_.nodes_.size());

  auto const adjust_section_element = [&](auto&& e) {
    if (e->is(type::CROSS)) {
      result[e->node(cross::nodes::sl_to_el)->id_] =
          element_coordinates[e->get_id()] +
          CROSS_OFFSETS.at(cross::nodes::sl_to_el);
      result[e->node(cross::nodes::el_to_sl)->id_] =
          element_coordinates[e->get_id()] +
          CROSS_OFFSETS.at(cross::nodes::el_to_sl);
      result[e->node(cross::nodes::sr_to_er)->id_] =
          element_coordinates[e->get_id()] +
          CROSS_OFFSETS.at(cross::nodes::sr_to_er);
      result[e->node(cross::nodes::er_to_sr)->id_] =
          element_coordinates[e->get_id()] +
          CROSS_OFFSETS.at(cross::nodes::er_to_sr);
      return;
    }

    for (auto const [i, n] : utl::enumerate(e->nodes())) {
      result[n->id_] = element_coordinates[e->get_id()] + OFFSETS[i];
    }
  };

  for (auto const& section : infra.graph_.sections_) {
    adjust_section_element(section.first_rising());
    adjust_section_element(section.last_rising());

    for (auto const& e : section.iterate<mileage_dir::rising, skip::No>()) {
      if (!e->is_track_element()) {
        continue;
      }

      auto const& te = e->as<track_element>();
      result[te.node()->id_] = element_coordinates[te.id_] +
                               (is_rising(te.dir()) ? top_offset : bot_offset);
    }
  }

  // every node has a coordinate assigned
  utls::ensure(result.size() == infra.graph_.nodes_.size());
  utls::ensure(utls::all_of(result, [](auto&& e) { return e.valid(); }));

  return result;
}

}  // namespace soro::infra
