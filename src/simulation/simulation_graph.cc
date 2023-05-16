#include "soro/simulation/simulation_graph.h"

#include "soro/infrastructure/graph/type.h"

#include "soro/utls/container/it_range.h"
#include "soro/utls/narrow.h"

namespace soro::simulation {

using namespace soro::tt;
using namespace soro::infra;

simulation_graph::node::id simulation_graph::node::get_id(
    soro::simulation::simulation_graph const& sg) const {
  utls::expect(!sg.nodes_.empty(), "simulation graph has no nodes");
  utls::expect(this >= sg.nodes_.data(), "node is not in simulation graph");
  utls::expect(this <= sg.nodes_.data() + sg.nodes_.size() - 1);

  return utls::narrow<id>(this - sg.nodes_.data());
}

simulation_graph::node::id find_element_in_ordering_group(
    element::ptr const e,
    std::span<simulation_graph::node> const ordering_group,
    simulation_graph const& sg, infrastructure const& infra) {

  auto const it = utls::find_if(ordering_group, [&](auto&& node) {
    auto const& element = infra->graph_.elements_[node.element_id_];
    return element->id() == e->id();
  });

  utls::sassert(it != std::end(ordering_group));

  return it->get_id(sg);
}

simulation_graph::node::id find_signal_eotd_in_ordering_group(
    std::span<simulation_graph::node> const ordering_group,
    simulation_graph const& sg, infrastructure const& infra) {

  auto const it = utls::find_if(ordering_group, [&](auto&& node) {
    auto const& element = infra->graph_.elements_[node.element_id_];
    return element->is(type::EOTD);
  });

  utls::sasserts([&] {
    auto const element = infra->graph_.elements_[it->element_id_];
    utls::sassert(infra->graph_.element_data<eotd>(element).signal_,
                  "first eotd in ordering group is not a signal eotd");
  });

  return it->get_id(sg);
}

void construct_nodes(simulation_graph* sg, infrastructure const& infra,
                     timetable const&, ordering_graph const& og) {

  static const type_set simulation_node_types = {
      type::MAIN_SIGNAL, type::APPROACH_SIGNAL, type::EOTD, type::HALT};

  // TODO(julian) don't iterate over a map, use a vector instead
  // we don't need the trips here anyways
  for (auto const& [trip, trip_nodes] : og.trip_to_nodes_) {
    soro::size_t const trip_start = sg->nodes_.size();

    for (auto i = trip_nodes.first; i < trip_nodes.second; ++i) {
      auto const& ordering_node = og.nodes_[i];
      auto const& ir = infra->interlocking_.routes_[ordering_node.ir_id_];

      // TODO(julian) these can be generated in a preprocessing step, when
      // creating the interlocking routes
      soro::size_t const start = sg->nodes_.size();
      for (auto const& rn : ir.iterate(infra)) {
        if (simulation_node_types.contains(rn.node_->type())) {
          sg->nodes_.emplace_back(rn.node_->element_->id());
        }
      }
      soro::size_t const end = sg->nodes_.size();

      sg->ordering_groups_.emplace_back(std::span{
          std::begin(sg->nodes_) + start, std::begin(sg->nodes_) + end});
    }

    soro::size_t const trip_end = sg->nodes_.size();
    sg->trips_.emplace_back(std::span{std::begin(sg->nodes_) + trip_start,
                                      std::begin(sg->nodes_) + trip_end});
  }
}

void construct_edges(simulation_graph*, infrastructure const&, timetable const&,
                     ordering_graph const&) {

  //  // TODO(julian) don't iterate over a map, use a vector instead
  //  // we don't need the trips here anyways
  //  for (auto const& [trip, trip_nodes] : og.trip_to_nodes_) {
  //    utls::it_range const node_range{std::begin(og.nodes_) +
  //    trip_nodes.first,
  //                                    std::begin(og.nodes_) +
  //                                    trip_nodes.second};
  //
  //    for (auto const [from, to] : utl::pairwise(node_range)) {
  //
  //      //
  //      //          v---- in
  //      // from -> to
  //
  //      for (auto const in : to.in_) {
  //        if (from.id_ == in) {
  //          continue;
  //        }
  //
  //        auto const& in_node = og.nodes_[in];
  //
  //        auto const excl = exclusion_graph.nodes_[in_node.ir_id_][to.ir_id_];
  //
  //        if (excl.node_offset_.has_value()) {
  //          auto const& in_ir = infra->interlocking_.routes_[in_node.ir_id_];
  //          auto const element = in_ir.nth_element(*excl.node_offset_, infra);
  //
  //          find_element_in_interlocking_group(element, sg, infra);
  //          auto const from_node = find_element_in_sim_node(infra_node, );
  //
  //          // add from -> to
  //
  //        } else {
  //
  //          auto const after_in = in_node.next(og);
  //          auto const from_node = find_signal_eotd_in_ordering_group(
  //              sg->ordering_groups_[after_in.id_], sg, infra);
  //
  //          // add normal edge
  //        }
  //      }
  //    }
  //  }
  //
  //  // generate edges
  //  for (auto const& train : sg.trains_) {
  //
  //    for (auto const& ir_group : train.interlocking_groups_) {
  //      for (auto const& node_id : ir_group.nodes_) {
  //
  //        auto const& node = infra->graph_.nodes_[sg.nodes_[node_id]];
  //        auto const& element = node->element_;
  //
  //        type_set in_types = {type::MAIN_SIGNAL, type::APPROACH_SIGNAL};
  //
  //        switch (element->type()) {
  //          case type::MAIN_SIGNAL:
  //          case type::APPROACH_SIGNAL:
  //          case type::HALT: break;
  //
  //          default:
  //        }
  //
  //        if (in_types.contains(element->type())) {
  //          // needs an in edge
  //        }
  //      }
  //    }
  //  }
}

simulation_graph::simulation_graph(infrastructure const& infra,
                                   timetable const& timetable,
                                   ordering_graph const& og) {
  construct_nodes(this, infra, timetable, og);
  construct_edges(this, infra, timetable, og);
}

}  // namespace soro::simulation
