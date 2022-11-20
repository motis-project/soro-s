#include "doctest/doctest.h"

#include "fmt/format.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/exclusion.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "test/file_paths.h"

using namespace soro;
using namespace infra;

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

  void interlocking_route_path_is_valid(interlocking_route const& ir,
                                        infrastructure const& infra) {
    CHECK(ir.first_sr(infra)->can_start_an_interlocking(
        infra->station_route_graph_));
    CHECK(ir.last_sr(infra)->can_end_an_interlocking(
        infra->station_route_graph_));

    CHECK_LT(ir.start_offset_, ir.first_sr(infra)->size());
    CHECK_LT(ir.end_offset_, ir.last_sr(infra)->size());

    CHECK(ir.valid_ends().contains(ir.first_node(infra)->type()));
    CHECK(ir.valid_ends().contains(ir.last_node(infra)->type()));
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

  TEST_CASE("interlocking routes de_iss") {
    auto opts = DE_ISS_OPTS;
    infrastructure const infra(opts);
    //    check_interlocking_routes(infra);
  }
}
