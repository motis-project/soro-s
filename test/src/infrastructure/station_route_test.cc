#include "test/infrastructure/graph_test.h"

#include "doctest/doctest.h"

#include "soro/utls/coroutine/collect.h"

#include "soro/infrastructure/path/length.h"

#include "test/utls/utls.h"

namespace soro::infra::test {

void check_station_route_members(station_route::ptr const sr) {
  CHECK(station_route::valid(sr->id_));

  CHECK_NE(sr->path_, nullptr);

  CHECK(!sr->nodes().empty());
  CHECK(!sr->name_.empty());

  CHECK_NE(sr->path_->start_, nullptr);
  CHECK_NE(sr->path_->end_, nullptr);

  REQUIRE_NE(sr->station_, nullptr);

  if (sr->nodes().back()->next_node_ != nullptr && !sr->is_in_route() &&
      !sr->is_contained_route()) {
    CHECK(sr->to_station_.has_value());
  }

  if (!sr->nodes().front()->reverse_edges_.empty() && !sr->is_out_route() &&
      !sr->is_contained_route()) {
    CHECK(sr->from_station_.has_value());
  }

  CHECK(si::valid(sr->length_));
}

void check_station_route_iteration(station_route::ptr const sr) {
  auto no_skipped_nodes = utls::collect<std::vector<node::ptr>>(
      utls::coro_map(sr->iterate(), [](auto&& rn) { return rn.node_; }));

  CHECK_EQ(no_skipped_nodes.size(), sr->size());
}

void check_station_route_length(station_route::ptr const sr) {
  auto const e1 = get_path_length_from_elements(sr->nodes());
  auto const e2 = get_path_length_from_elements(
      utls::coro_map(sr->iterate(), [](auto&& rn) { return rn.node_; }));

  CHECK_EQ(e1, e2);

  auto const s1 = get_path_length_from_sections(sr->nodes());
  auto const s2 = get_path_length_from_sections(
      utls::coro_map(sr->iterate(), [](auto&& rn) { return rn.node_; }));

  CHECK_EQ(s1, s2);

  CHECK_MESSAGE((e1 == s2),
                "Different lengths from the two length calculation funs");
  CHECK_MESSAGE((sr->length_ == e1),
                "Different length from tests to station route member.");
}

void check_station_routes(infrastructure const& infra) {
  soro::test::utls::check_continuous_ascending_ids(infra->station_routes_);

  for (auto const& station_route : infra->station_routes_) {
    check_station_route_iteration(station_route);
    check_station_route_length(station_route);
    check_station_route_members(station_route);
  }
}

void do_station_route_tests(infrastructure const& infra) {
  check_station_routes(infra);
}

}  // namespace soro::infra::test
