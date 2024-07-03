#include "test/timetable/train_iterator_test.h"

#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

#include "doctest/doctest.h"

#include "utl/erase_duplicates.h"
#include "utl/pairwise.h"

#include "soro/utls/any.h"
#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/node.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/station/station_route.h"

#include "soro/timetable/train.h"

using namespace soro::infra;

namespace soro::tt::test {

void check_element_uniqueness(train const& t, infrastructure const& infra) {
  std::set<element_id> elements;

  for (auto const& rn : t.iterate(infra)) {
    if (rn.omitted_) {
      continue;
    }

    auto const& [ignore, success] =
        elements.emplace(rn.node_->element_->get_id());
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

  for (auto const& tn : t.iterate(infra)) {
    train_path_elements.insert(tn.node_->id_);
  }

  utls::sassert(t.break_in_ || t.sequence_points_.front().is_halt(),
                "first sp non halt requires break in, not supported otherwise");

  bool found_first = t.break_in_;
  for (auto const& ir_id : t.path_) {
    auto const& ir = infra->interlocking_.routes_[ir_id];

    for (auto const& rn : ir.iterate(infra)) {
      found_first |= t.sequence_points_.front().get_node(infra) == rn.node_;

      if (!found_first) continue;

      interlocking_route.insert(rn.node_->id_);
    }
  }

  if (!t.break_in_) {
    auto const& sp = t.sequence_points_.front();
    auto const& ir = t.first_interlocking_route(infra);

    auto g = ir.to(sp.station_route_, sp.idx_, infra);

    for (auto&& rn : g) {
      interlocking_route.erase(rn.node_->id_);
    }
  }

  if (!t.break_out_) {
    auto const& sp = t.sequence_points_.back();
    auto const& ir = t.last_interlocking_route(infra);

    auto g = ir.from(sp.station_route_, sp.idx_ + 1, infra);

    for (auto&& rn : g) {
      interlocking_route.erase(rn.node_->id_);
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

    if (tn.omitted() && !tn.omitted_) {
      // train omitted nodes must be halts and can't have sequence points
      CHECK(tn.node_->is(type::HALT));
      CHECK(!tn.sequence_point_.has_value());
    }
  }

  for (auto const& ir_id : t.path_) {
    auto const& ir = infra->interlocking_.routes_[ir_id];
    for (auto const& sr_id : ir.station_routes_) {
      auto const& sr = infra->station_routes_[sr_id];
      for (auto const omitted : sr->omitted_nodes_) {
        sr_omitted.emplace_back(sr->nodes(omitted)->id_);
      }
    }
  }

  // no duplicates in train omitted nodes
  auto const old_train_size = train_omitted.size();
  utl::erase_duplicates(train_omitted);
  CHECK_EQ(old_train_size, train_omitted.size());

  utl::erase_duplicates(sr_omitted);
  std::vector<node::id> train_omitted_without_sr_omitted;
  std::set_difference(std::begin(train_omitted), std::end(train_omitted),
                      std::begin(sr_omitted), std::end(sr_omitted),
                      std::back_inserter(train_omitted_without_sr_omitted));
  CHECK_MESSAGE(
      train_omitted_without_sr_omitted.empty(),
      "Train can't have more omitted nodes than the used station routes.");

  // TODO(julian) To fully check for omitted nodes we have to check the
  // nodes that come before the train start and after the train end.
  // i.e. sr_omitted_without_train_omitted can only appear before the first
  // train node and after the last train node
}

void check_every_train_sequence_point_has_node(train const& t) {
  for (auto const& sp : t.sequence_points_) {
    CHECK_NE(sp.idx_, station_route::invalid_idx());
  }
}

void check_breaking_in_trains(train const& t, infrastructure const& infra) {
  auto const first_element = t.get_start_node(infra)->element_;
  auto const first_iterated = *t.iterate(infra).begin();

  CHECK_EQ(first_element->get_id(), first_iterated.node_->element_->get_id());

  // TODO(julian) might not necessarily be true
  CHECK((!t.break_in_ || first_element->type() == type::TRACK_END));

  if (first_element->type() == utls::any{type::TRACK_END, type::BORDER}) {
    CHECK_MESSAGE(t.break_in_,
                  "train does not start on halt, but it is not breaking in");
  }
}

void check_breaking_out_trains(train const& t, infrastructure const& infra) {
  auto const last_ir = infra->interlocking_.routes_[t.path_.back()];
  auto const last_element = t.get_end_node(infra)->element_;

  element::ptr last_iterated = nullptr;
  for (auto&& tn : t.iterate(infra)) {
    last_iterated = tn.node_->element_;
  }

  CHECK_EQ(last_element->get_id(), last_iterated->get_id());

  // TODO(julian) might not necessarily be true
  CHECK((!t.break_out_ || last_element->type() == type::TRACK_END));

  if (last_element->type() == utls::any{type::TRACK_END, type::BORDER}) {
    CHECK_MESSAGE(t.break_out_,
                  "train does not end on halt, but it is not breaking out");
  }
}

void do_train_iterator_tests(train const& t, infrastructure const& infra) {
  check_element_uniqueness(t, infra);
  check_node_uniqueness(t, infra);
  check_train_path_connections(t, infra);
  check_every_train_sequence_point_has_node(t);
  compare_train_path_elements_with_ir_elements(t, infra);
  check_omitted_nodes(t, infra);
  check_breaking_in_trains(t, infra);
  check_breaking_out_trains(t, infra);
}

}  // namespace soro::tt::test
