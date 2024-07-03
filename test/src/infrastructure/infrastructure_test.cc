#include "doctest/doctest.h"

#include <cstddef>

#include "soro/utls/execute_if.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/accumulate.h"
#include "soro/utls/std_wrapper/find_if.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/section.h"
#include "soro/infrastructure/infrastructure.h"

#include "test/file_paths.h"
#include "test/infrastructure/graph_test.h"
#include "test/infrastructure/interlocking_route_test.h"
#include "test/infrastructure/reverse_ahead_test.h"
#include "test/infrastructure/section_test.h"
#include "test/infrastructure/station_route_graph_test.h"
#include "test/infrastructure/station_route_test.h"
#include "test/utls/utls.h"

#if defined(SERIALIZE)
#include "cista/serialization.h"
#endif

using namespace soro::si;
using namespace soro::utls;
using namespace soro::infra;

namespace soro::infra::test {

void check_ascending_ids(infrastructure const& infra) {
  soro::test::utls::check_continuous_ascending_ids(infra->stations_);
}

void check_speed_limit_values(infrastructure const& infra) {
  // if a special speed limit band is ending, it does not need a speed value
  for (auto const& data : infra->graph_.element_data_) {
    execute_if<speed_limit>(data, [](auto&& spl) {
      if (spl.type_ != speed_limit::type::end_special) {
        CHECK(spl.limit_.is_valid());
      }
    });
  }
}

void check_border_number(infrastructure const& infra) {
  if (infra->stations_.size() == 1) {
    return;  // no border checking with a single station
  }

  auto const total_borders = utls::accumulate(
      infra->stations_, std::size_t{0},
      [](auto&& acc, auto&& s) { return acc + s->borders_.size(); });

  CHECK_MESSAGE((total_borders != 0),
                "if there is more than one station we need realized borders");
}

void check_border_pairs(infrastructure const& infra) {
  for (auto const& station_a : infra->stations_) {
    for (auto const& border_a : station_a->borders_) {
      auto const border_b =
          *utls::find_if(border_a.neighbour_->borders_, [&](auto&& border) {
            return border.get_id_tuple() == border_a.get_id_tuple();
          });

      CHECK_MESSAGE((border_a.pos_ != section::position::middle),
                    "borders must have position");
      CHECK_MESSAGE((border_b.pos_ != section::position::middle),
                    "borders must have position");
      bool const matching_orientation = border_a.pos_ != border_b.pos_;
      CHECK_MESSAGE(matching_orientation,
                    "two connected borders should always be different");
    }
  }
}

void check_orientation_flags_on_track_elements(infrastructure const& infra) {
  auto const check_node_line = [](node_ptr node) {
    while (node->next_ != nullptr &&
           node->next_->element_->is_track_element()) {
      utls::sassert(node->element_->is_track_element());
      utls::sassert(node->next_->element_->is_track_element());

      auto const& from = node->element_->as<track_element>();
      auto const& to = node->next_->element_->as<track_element>();

      CHECK_MESSAGE(
          (from.dir() == to.dir()),
          "consecutive track elements must have the same orientation");

      node = node->next_;
    }
  };

  for (auto const& element : infra->graph_.elements_) {
    if (element->is_track_element()) {
      continue;
    }

    for (auto const& node : element->nodes()) {
      if (node->next_ != nullptr && node->next_->element_->is_track_element()) {
        check_node_line(node->next_);
      }

      if (node->branch_ != nullptr &&
          node->branch_->element_->is_track_element()) {
        check_node_line(node->branch_);
      }
    }
  }
}

void check_infra(infrastructure const& infra) {
  SUBCASE("speed limit values tests") { check_speed_limit_values(infra); }

  SUBCASE("graph tests") { check_graph(infra->graph_); }

  SUBCASE("track element tests") {
    check_orientation_flags_on_track_elements(infra);
  }

  SUBCASE("ascending ids tests") { check_ascending_ids(infra); }

  SUBCASE("section tests") { do_section_tests(infra->graph_.sections_); }

  SUBCASE("station route tests") { do_station_route_tests(infra); }
  SUBCASE("station route graph tests") { do_station_route_graph_tests(infra); }

  SUBCASE("interlocking route tests") { do_interlocking_route_tests(infra); }

  SUBCASE("border tests") {
    check_border_pairs(infra);
    check_border_number(infra);
  }

  SUBCASE("reverse ahead tests") { do_reverse_ahead_tests(infra); }
}

TEST_CASE("parse infrastructure") {
  for (auto const& not_serialized :
       soro::test::get_infrastructure_scenarios()) {
#if defined(SERIALIZE)
    not_serialized->save("test.raw");
    infrastructure const infra("test.raw");
#else
    auto const& infra = *not_serialized;
#endif

    check_infra(infra);
  }
}

}  // namespace soro::infra::test
