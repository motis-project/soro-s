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
    element_id const e, simulation_graph::ordering_group const ordering_group,
    simulation_graph const& sg) {

  auto const it = utls::find_if(
      ordering_group, [&](auto&& node) { return node.element_id_ == e; });

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

      sg->ordering_groups_.emplace_back(std::begin(sg->nodes_) + start,
                                        std::begin(sg->nodes_) + end);
    }

    soro::size_t const trip_end = sg->nodes_.size();
    sg->trips_.emplace_back(std::begin(sg->nodes_) + trip_start,
                            std::begin(sg->nodes_) + trip_end);
  }
}

void construct_edges(simulation_graph* sg, infrastructure const& infra,
                     timetable const& tt, ordering_graph const& og) {

  std::ignore = sg;
  std::ignore = infra;
  std::ignore = tt;
  std::ignore = og;

  // TODO(julian) don't iterate over a map, use a vector instead
  // we don't need the trips here anyways
  for (auto const& [trip, trip_nodes] : og.trip_to_nodes_) {
    utls::it_range const node_range{std::begin(og.nodes_) + trip_nodes.first,
                                    std::begin(og.nodes_) + trip_nodes.second};

    for (auto const [from, to] : utl::pairwise(node_range)) {
      std::vector<simulation_graph::node::id> in_edges;

      //
      //          v---- in
      // from -> to

      for (auto const in : to.in_) {
        if (from.id_ == in) {
          continue;
        }

        auto const& in_node = og.nodes_[in];

        auto const& ex = infra->exclusion_.exclusion_graph_
                             .data_[from.ir_id_][in_node.ir_id_];

        if (ex.route_eotd_.has_value()) {
          auto const from_node = find_element_in_ordering_group(
              *ex.route_eotd_, sg->ordering_groups_[in], *sg);

          in_edges.emplace_back(from_node);
        } else {

          auto const after_in = in_node.next(og);
          auto const from_node = find_signal_eotd_in_ordering_group(
              sg->ordering_groups_[after_in.id_], *sg, infra);

          in_edges.emplace_back(from_node);
        }
      }

      sg->in_.emplace_back(in_edges);
    }
  }
}

simulation_graph::simulation_graph(infrastructure const& infra,
                                   timetable const& timetable,
                                   ordering_graph const& og) {
  construct_nodes(this, infra, timetable, og);
  construct_edges(this, infra, timetable, og);
}

}  // namespace soro::simulation
