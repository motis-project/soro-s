#include "test/timetable/train_iterator_test.h"

#include "utl/pipes.h"

#include "doctest/doctest.h"

#include <set>

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

namespace soro::tt::test {

void check_element_uniqueness(train const& t, infrastructure const& infra) {
  std::set<element_id> elements;

  for (auto const& rn : t.iterate(infra)) {
    if (rn.omitted_) {
      continue;
    }

    auto const& [ignore, success] = elements.emplace(rn.node_->element_->id());
    CHECK_MESSAGE(success,
                  "A train iterator should visit an element only once.");
  }
}

void check_node_uniqueness(train const& t, infrastructure const& infra) {
  std::set<node::id> nodes;
  for (auto const& rn : t.iterate(infra)) {
    if (rn.omitted_) {
      continue;
    }

    auto const& [ignore, success] = nodes.emplace(rn.node_->id_);
    CHECK_MESSAGE(success, "A train iterator should visit a node only once.");
  }
}

void check_train_path_connections(train const& t, infrastructure const& infra) {
  for (auto const [ir_id1, ir_id2] : utl::pairwise(t.path_)) {
    auto const& ir1 = infra->interlocking_.routes_[ir_id1];
    auto const& ir2 = infra->interlocking_.routes_[ir_id2];

    CHECK_EQ(ir1.last_node(infra), ir2.first_node(infra));
  }
}

void compare_train_path_elements_with_ir_elements(train const& t,
                                                  infrastructure const& infra) {
  std::set<node::id> train_path_elements;
  std::set<node::id> interlocking_route;

  for (auto tn : t.iterate(infra)) {
    train_path_elements.insert(tn.node_->id_);
  }

  for (auto const& ir : t.path(infra)) {
    for (auto const& rn : ir.iterate(infra)) {
      interlocking_route.insert(rn.node_->id_);
    }
  }

  CHECK_EQ(train_path_elements, interlocking_route);
}

void check_omitted_nodes(train const& t, infrastructure const& infra) {
  std::vector<node::id> train_omitted;
  std::vector<node::id> sr_omitted;

  for (auto const& tn : t.iterate(infra)) {
    if (tn.omitted_) {
      train_omitted.emplace_back(tn.node_->id_);
    }
  }

  for (auto const& ir : t.path(infra)) {
    for (auto const& subpath : ir.iterate_station_routes(*infra)) {
      for (auto const omitted : subpath.station_route_->omitted_nodes_) {
        if (subpath.from_ <= omitted && omitted < subpath.to_) {
          sr_omitted.emplace_back(subpath.station_route_->nodes(omitted)->id_);
        }
      }
    }
  }

  CHECK_EQ(train_omitted, sr_omitted);
}

// TODO(julian) Check for correctly omitted nodes

void check_every_train_sequence_point_has_node(train const& t,
                                               infrastructure const& infra) {
  for (auto const& sp : t.sequence_points_) {
    CHECK(sp.get_node(t.freight(), infra).has_value());
  }
}

void do_train_iterator_tests(train const& t, infrastructure const& infra) {
  check_element_uniqueness(t, infra);
  check_node_uniqueness(t, infra);
  check_train_path_connections(t, infra);
  check_every_train_sequence_point_has_node(t, infra);
  compare_train_path_elements_with_ir_elements(t, infra);
  check_omitted_nodes(t, infra);
}

}  // namespace soro::tt::test
