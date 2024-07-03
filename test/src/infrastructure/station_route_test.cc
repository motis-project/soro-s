#include "doctest/doctest.h"

#include <cstddef>
#include <iterator>
#include <tuple>
#include <vector>

#include "utl/concat.h"

#include "soro/utls/coroutine/collect.h"
#include "soro/utls/coroutine/coro_map.h"
#include "soro/utls/narrow.h"
#include "soro/utls/std_wrapper/find.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/std_wrapper/sort.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/length.h"
#include "soro/infrastructure/station/station_route.h"

#include "test/utls/utls.h"

namespace soro::infra::test {

void check_station_route_members(station_route::ptr const sr) {
  CHECK_NE(sr->id_, station_route::invalid());

  CHECK_NE(sr->path_, nullptr);

  CHECK(!sr->nodes().empty());
  CHECK(!sr->name_.empty());

  CHECK_NE(sr->path_->start_, nullptr);
  CHECK_NE(sr->path_->end_, nullptr);

  REQUIRE_NE(sr->station_, nullptr);

  if (sr->nodes().back()->next_ != nullptr && !sr->is_in_route() &&
      !sr->is_contained_route()) {
    CHECK(sr->to_station_.has_value());
  }

  if (!sr->nodes().front()->reverse_edges_.empty() && !sr->is_out_route() &&
      !sr->is_contained_route()) {
    CHECK(sr->from_station_.has_value());
  }

  CHECK(sr->length_.is_valid());
}

void check_no_nodes_skipped(station_route::ptr const sr) {
  auto const no_skipped_nodes_fwd = utls::collect<std::vector<node::ptr>>(
      utls::coro_map(sr->iterate(), [](auto&& rn) { return rn.node_; }));

  auto const no_skipped_nodes_bwd = utls::collect<std::vector<node::ptr>>(
      utls::coro_map(sr->backwards(), [](auto&& rn) { return rn.node_; }));

  CHECK_EQ(no_skipped_nodes_fwd.size(), sr->size());
  CHECK_EQ(no_skipped_nodes_bwd.size(), sr->size());
}

void check_omitted_and_extra_speed_limits(station_route::ptr const sr) {
  std::size_t total_omitted = 0;
  std::size_t total_extra = 0;

  station_route::idx idx = 0;
  for (auto const& rn : sr->iterate()) {
    if (rn.omitted_) {
      CHECK_NE(utls::find(sr->omitted_nodes_, idx),
               std::end(sr->omitted_nodes_));
      ++total_omitted;
    }

    for (auto const& spl : rn.extra_spls_) {
      std::ignore = spl;

      CHECK_NE(utls::find_if(sr->extra_speed_limits_,
                             [&](auto&& extra) { return extra.idx_ == idx; }),
               std::end(sr->extra_speed_limits_));
      ++total_extra;
    }

    ++idx;
  }

  CHECK_EQ(total_omitted, sr->omitted_nodes_.size());
  CHECK_EQ(total_extra, sr->extra_speed_limits_.size());
}

void check_fwd_and_bwd_equal(station_route::ptr const sr) {
  auto fwd = utls::collect<std::vector<route_node>>(
      utls::coro_map(sr->iterate(), [](auto&& rn) { return rn; }));

  auto bwd = utls::collect<std::vector<route_node>>(
      utls::coro_map(sr->backwards(), [](auto&& rn) { return rn; }));

  CHECK_EQ(fwd.size(), bwd.size());

  for (auto idx = 0U; idx < fwd.size(); ++idx) {
    // sort the speed limits, since they are not yielded in the same order
    // when iterating forward or backward.
    utls::sort(fwd[idx].extra_spls_);
    utls::sort(bwd[bwd.size() - idx - 1].extra_spls_);

    CHECK_EQ(fwd[idx], bwd[bwd.size() - idx - 1]);
  }
}

void check_half_and_half(station_route::ptr const sr) {
  auto const half = utls::narrow<station_route::idx>(sr->size() / 2);

  auto fwd = utls::collect<std::vector<route_node>>(sr->from_to(0, half));
  auto fwd2 =
      utls::collect<std::vector<route_node>>(sr->from_to(half, sr->size()));

  auto bwd =
      utls::collect<std::vector<route_node>>(sr->from_to(sr->size() - 1, half));
  auto bwd2 = utls::collect<std::vector<route_node>>(sr->from_to(half, -1));

  utl::concat(fwd, fwd2);
  utl::concat(bwd, bwd2);

  for (auto idx = 0U; idx < fwd.size(); ++idx) {
    // sort the speed limits, since they are not yielded in the same order
    // when iterating forward or backward.
    utls::sort(fwd[idx].extra_spls_);
    utls::sort(bwd[bwd.size() - idx - 1].extra_spls_);

    CHECK_EQ(fwd[idx], bwd[bwd.size() - idx - 1]);
  }
}

void check_station_route_iteration(station_route::ptr const sr) {
  check_omitted_and_extra_speed_limits(sr);
  check_no_nodes_skipped(sr);
  check_fwd_and_bwd_equal(sr);
  check_half_and_half(sr);
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

void check_station_route_speed_limits(station_route::ptr const sr) {
  for (auto const& extra_spl : sr->extra_speed_limits_) {
    auto const& spl = extra_spl.spl_;
    CHECK_MESSAGE(spl.from_station_route(), "speed limit source not correct");
    CHECK_MESSAGE(spl.length_.is_valid(), "got invalid length in sr limit");
    // length l must be: 0 < l <= max or be inf
    CHECK_GT(spl.length_, si::length::zero());
    auto const is_inf = spl.length_.is_infinity();
    auto const less_than_max = spl.length_ < si::length::max();
    CHECK((is_inf || less_than_max));
  }
}

void check_station_routes(infrastructure const& infra) {
  soro::test::utls::check_continuous_ascending_ids(infra->station_routes_);

  for (auto const& station_route : infra->station_routes_) {
    check_station_route_iteration(station_route);
    check_station_route_length(station_route);
    check_station_route_members(station_route);
    check_station_route_speed_limits(station_route);
  }
}

void do_station_route_tests(infrastructure const& infra) {
  check_station_routes(infra);
}

}  // namespace soro::infra::test
