#include "test/infrastructure/interlocking_route_test.h"

#include "doctest/doctest.h"

#include "fmt/format.h"
#include "utl/enumerate.h"

#include "soro/utls/coroutine/coro_map.h"
#include "soro/utls/graph/traversal.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/interlocking/exclusion.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/infrastructure/path/is_path.h"
#include "soro/infrastructure/path/length.h"

namespace soro::infra::test {

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
      (ssr_count <= infra->interlocking_.routes_.size()),
      fmt::format("Exptected at least {} signal station routes, but got {}",
                  ssr_count, infra->interlocking_.routes_.size()));
}

void check_station_route_exclusions(infrastructure const& infra) {
  auto const station_route_exclusions =
      detail::get_station_route_exclusions(*infra);

  for (auto const [id, exclusions] : utl::enumerate(station_route_exclusions)) {
    for (auto const excluded_route : exclusions) {
      CHECK(utls::contains(station_route_exclusions[excluded_route], id));
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

  std::vector<element::ptr> elements;

  for (auto const rn : ir.iterate(infra)) {
    elements.emplace_back(rn.node_->element_);
  }

  CHECK_MESSAGE(is_path(elements), "Interlocking route is not a path.");
}

void check_interlocking_route(interlocking_route const& ir,
                              infrastructure const& infra) {
  interlocking_route_path_is_valid(ir, infra);
}

void check_interlocking_routes(infrastructure const& infra) {
  for (auto const& ir : infra->interlocking_.routes_) {
    check_interlocking_route(ir, infra);
  }
}

void check_interlocking_route_lengths(infrastructure const& infra) {
  for (auto const& ir : infra->interlocking_.routes_) {
    auto const e1 = get_path_length_from_elements(
        utls::coro_map(ir.iterate(infra), [](auto&& rn) { return rn.node_; }));

    auto const s1 = get_path_length_from_sections(
        utls::coro_map(ir.iterate(infra), [](auto&& rn) { return rn.node_; }));

    CHECK_MESSAGE((e1 == s1),
                  "Different lengths from the two length calculation funs");
    CHECK_EQ(ir.length_, e1);
  }
}

void check_halting_at(infrastructure const& infra) {
  for (node::id node_id = 0; node_id < infra->interlocking_.halting_at_.size();
       ++node_id) {
    for (auto const ir_id : infra->interlocking_.halting_at_[node_id]) {
      auto const& ir = infra->interlocking_.routes_[ir_id];

      CHECK(utls::contains_if(ir.iterate(infra), [&](auto&& rn) {
        return rn.node_->id_ == node_id;
      }));
    }
  }
}

void check_starting_at(infrastructure const& infra) {
  for (auto const [node_id, irs] :
       utl::enumerate(infra->interlocking_.starting_at_)) {
    for (auto const ir_id : irs) {
      auto const& ir = infra->interlocking_.routes_[ir_id];
      CHECK_EQ(ir.first_node(infra)->id_, node_id);
    }
  }
}

void do_interlocking_route_tests(infrastructure const& infra) {
  check_interlocking_route_count(infra);
  check_interlocking_routes(infra);

  check_interlocking_route_exclusions(infra->interlocking_);
  check_station_route_exclusions(infra);
  check_interlocking_route_lengths(infra);

  check_halting_at(infra);
  check_starting_at(infra);
}

}  // namespace soro::infra::test
