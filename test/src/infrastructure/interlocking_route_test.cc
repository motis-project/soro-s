#include "doctest/doctest.h"

#include "fmt/format.h"
#include "utl/enumerate.h"

#include "soro/utls/coroutine/coro_map.h"
#include "soro/utls/graph/traversal.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/exclusion.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/path/is_path.h"

#include "test/file_paths.h"

using namespace soro;
using namespace infra;

void check_interlocking_route_count(infrastructure const& infra) {
  soro::size_type inner_sr_count = 0;
  soro::size_type path_sr_count = 0;

  for (auto const& sr : infra->station_routes_) {
    if (sr->path_->main_signals_.empty()) {
      continue;
    }

    soro::size_type reachable_ms = 0;

    auto const& handle_node = [&](auto&& sr_ptr, auto&&) {
      if (sr_ptr != sr && !sr_ptr->path_->main_signals_.empty()) {
        ++reachable_ms;
      }

      return false;
    };

    auto const& get_neighbours = [&](auto&& sr_ptr) {
      if (sr_ptr == sr || sr_ptr->path_->main_signals_.empty()) {
        return infra->station_route_graph_.successors_[sr_ptr->id_];
      } else {
        return decltype(infra->station_route_graph_.successors_.front()){};
      }
    };

    utls::dfs(sr, handle_node, get_neighbours);

    path_sr_count += reachable_ms;

    if (sr->path_->main_signals_.size() > 1) {
      inner_sr_count += sr->path_->main_signals_.size() - 1;
    }
  }

  auto const ssr_count = inner_sr_count + path_sr_count;

  CHECK_MESSAGE(
      (ssr_count <= infra->interlocking_.interlocking_routes_.size()),
      fmt::format("Exptected at least {} signal station routes, but got {}",
                  ssr_count, infra->interlocking_.interlocking_routes_.size()));
}

void check_station_route_exclusions(infrastructure const& infra) {
  auto const station_route_exclusions =
      detail::get_station_route_exclusions(*infra);

  for (auto const [id, exclusions] : utl::enumerate(station_route_exclusions)) {
    for (auto const excluded_route : exclusions) {
      CHECK(!utls::contains(station_route_exclusions[excluded_route], id));
    }
  }
}

void check_interlocking_route_exclusions(interlocking_subsystem const& irs) {
  for (auto const [id, exclusions] : utl::enumerate(irs.exclusions_)) {
    for (auto const excluded_route : exclusions) {
      CHECK(!utls::contains(irs.exclusions_[excluded_route], id));
    }
  }
}

void interlocking_route_path_is_valid(interlocking_route const& ir,
                                      infrastructure const& infra) {
  CHECK_MESSAGE(ir.first_sr(infra)->can_start_an_interlocking(
                    infra->station_route_graph_),
                "First station route cannot start an interlocking route!");
  CHECK_MESSAGE(
      ir.last_sr(infra)->can_end_an_interlocking(infra->station_route_graph_),
      "Last station route cannot end an interlocking route.");

  // check if start and end offset don't violate the station routes sizes
  CHECK_LT(ir.start_offset_, ir.first_sr(infra)->size());
  CHECK_LT(ir.end_offset_, ir.last_sr(infra)->size());

  CHECK_MESSAGE(ir.valid_ends().contains(ir.first_node(infra)->type()),
                "Interlocking route does not start on a valid type");
  CHECK_MESSAGE(ir.valid_ends().contains(ir.last_node(infra)->type()),
                "Interlocking route does not end on a valid type");

  CHECK_MESSAGE(
      is_path(utls::coro_map(ir.iterate(skip_omitted::OFF, infra),
                             [](auto&& r) { return r.node_->element_; })),
      "Interlocking route is not a path.");
}

void check_interlocking_route(interlocking_route const& ir,
                              infrastructure const& infra) {
  interlocking_route_path_is_valid(ir, infra);
}

void check_interlocking_routes(infrastructure const& infra) {
  for (auto const& ir : infra->interlocking_.interlocking_routes_) {
    check_interlocking_route(ir, infra);
  }
}

TEST_SUITE("interlocking routes") {

  TEST_CASE("signal station route exclusion") {
    infrastructure const infra(SMALL_OPTS);

    // TODO(julian) Implement signal station route test here
    // compare exclusion results from CPU algorithm to GPU algorithm
  }

  TEST_CASE("signal station route get_exclusion_elements") {  // NOLINT
    infrastructure const infra(SMALL_OPTS);

    utls::sassert(false, "Not implemented");
    //  for (auto const& ssr : infra->interlocking_.interlocking_routes_) {
    //    auto const exclusion_elements = get_exclusion_elements(*ssr, *infra);
    //
    //    auto const& first_element = exclusion_elements.front();
    //    auto const& last_element = exclusion_elements.back();
    //
    //    CHECK_MESSAGE(
    //        interlocking_route::valid_ends().contains(first_element->type()),
    //        fmt::format("First element's type must be from the list of "
    //                    "valid elements, but was {}.",
    //                    first_element->get_type_str()));
    //    CHECK_MESSAGE(
    //        interlocking_route::valid_ends().contains(last_element->type()),
    //        fmt::format("Last element's type must be from the list of "
    //                    "valid elements, but was {}.",
    //                    last_element->get_type_str()));

    /*
     *  Every element on the path given by nodes_ must appear in the exclusion
     * element list (EEL). The EEL might contain more than the original path
     * elements. The original path elements in the EEL have to appear in the
     * same order as given in the nodes_ path
     */
    //    auto current_element_it = std::cbegin(exclusion_elements);
    //    for (auto const& node : ssr.iterate(infra)) {
    //      auto const& e = node->element_;
    //
    //      while (e->id() != (*current_element_it)->id()) {
    //        ++current_element_it;
    //      }
    //    }
    //
    //    CHECK_MESSAGE(current_element_it == std::cend(exclusion_elements) - 1,
    //                  "Could not find every original path element given by
    //                  nodes_");
    //  }
  }

  TEST_CASE("interlocking routes de_iss") {
    auto opts = DE_ISS_OPTS;
    infrastructure const infra(opts);

    check_interlocking_route_count(infra);
    check_interlocking_routes(infra);

    check_interlocking_route_exclusions(infra->interlocking_);
    check_station_route_exclusions(infra);
  }
}
