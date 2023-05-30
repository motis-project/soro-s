#include "soro/simulation/simulation_graph.h"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/type.h"

#include "soro/infrastructure/interlocking/interlocking_route.h"
#include "soro/timetable/sequence_point.h"
#include "soro/utls/container/it_range.h"
#include "soro/utls/narrow.h"
#include "soro/utls/std_wrapper/contains.h"

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

simulation_graph::node::id find_element_in_interlocking_group(
    element_id const e,
    simulation_graph::interlocking_group const interlocking_group,
    simulation_graph const& sg) {
  auto const it =
      utls::find_if(interlocking_group.skip_entry_ms(sg),
                    [&](auto&& node) { return node.element_id_ == e; });

  utls::sassert(it != std::end(interlocking_group.skip_entry_ms(sg)));

  return it->get_id(sg);
}

simulation_graph::node::id find_signal_eotd_in_interlocking_group(
    simulation_graph::interlocking_group const interlocking_group,
    simulation_graph const& sg, infrastructure const& infra) {

  auto const it =
      utls::find_if(interlocking_group.skip_entry_ms(sg), [&](auto&& node) {
        auto const& element = infra->graph_.elements_[node.element_id_];
        return element->is(type::EOTD);
      });

  utls::sasserts([&] {
    auto const element = infra->graph_.elements_[it->element_id_];
    utls::sassert(infra->graph_.element_data<eotd>(element).signal_,
                  "first eotd in interlocking group is not a signal eotd");
  });

  return it->get_id(sg);
}

utls::optional<element_id> get_route_eotd(interlocking_route const& from,
                                          interlocking_route const& to,
                                          infrastructure const& infra) {
  auto const& cps = infra->interlocking_.critical_points_;

  if (from.id_ == to.id_ || cps[from.id_].empty() || cps[to.id_].empty()) {
    return std::nullopt;
  }

  auto const from_start = from.get_start_critical_section(infra);
  auto const from_end = from.get_end_critical_section(infra);

  auto const to_start = to.get_start_critical_section(infra);
  auto const to_end = to.get_end_critical_section(infra);

  // from_ir has no critical points, we don't need to look for route eotds
  auto const from_has_no_cps = from_start == from_end;

  // to_ir has no critical points, we don't need to look for route eotds
  auto const to_has_no_cps = to_start == to_end;

  // both irs end in the same critical section, we don't need to look for route
  // eotds
  auto const same_end = from_end == to_end;

  // to_ir starts in the same critical section as from_ir ends, we don't need to
  // look for route eotds
  auto const from_goes_into_to = from_end == to_start;

  if (from_has_no_cps || to_has_no_cps || same_end || from_goes_into_to) {
    return std::nullopt;
  }

  element_id last_reotd = element::INVALID;
  for (auto idx = static_cast<std::ptrdiff_t>(cps[from.id_].size()) - 1;
       idx >= 0; --idx) {

    auto const& cp = cps[from.id_][idx];

    if (cp.type_ == type::EOTD) {
      last_reotd = cp.element_;
      continue;
    }

    auto const to_contains = utls::contains(cps[to.id_], cps[from.id_][idx]);

    if (to_contains) {
      return last_reotd == element::INVALID
                 ? utls::optional<element_id>{std::nullopt}
                 : soro::optional<element_id>{last_reotd};
    }
  }

  return std::nullopt;
}

utls::optional<element_id> get_route_eotd(interlocking_route::id const from,
                                          interlocking_route::id const to,
                                          infrastructure const& infra) {
  return get_route_eotd(infra->interlocking_.routes_[from],
                        infra->interlocking_.routes_[to], infra);
}

soro::vector<simulation_graph::train_dependency> get_train_dependencies(
    simulation_graph::node const& s_node,
    simulation_graph::interlocking_group const& ig, simulation_graph const& sg,
    infrastructure const& infra, ordering_graph const& og) {

  auto const& element = infra->graph_.elements_[s_node.element_id_];

  utls::sassert(
      element->is(type::MAIN_SIGNAL) || element->is(type::APPROACH_SIGNAL),
      "train dependency on {}", get_type_str(element->type()));

  auto const as = element->is(type::APPROACH_SIGNAL);
  auto const ms = element->is(type::MAIN_SIGNAL);

  auto const as_before_halt = as && ig.has_halt() && ig.approach_before_halt();
  auto const ms_without_halt = ms && !ig.has_halt();
  auto const ms_and_as_after_halt =
      ms && ig.has_halt() && ig.has_approach() && !ig.approach_before_halt();

  if (as_before_halt || ms_without_halt || ms_and_as_after_halt) {
    return {};
  }

  /*
   * we will inspect elements belonging to at most 4 different ordering nodes /
   * interlocking routes, for each dependency:
   *
   *              +-- [incoming_ordering_node] -> [next_incoming_ordering_node]
   *              |
   *              + ------------+
   *                            v
   *  [ordering_node] -> [next_ordering_node]
   *
   */

  soro::vector<simulation_graph::train_dependency> dependencies;

  auto const& ordering_node =
      og.nodes_[sg.simulation_node_to_ordering_node_[s_node.get_id(sg)]];

  // last interlocking route in the train run, we don't have any dependencies
  // anymore
  if (!ordering_node.has_next(og)) {
    return {};
  }

  auto const& next_ordering_node = ordering_node.next(og);

  for (auto const in_id : next_ordering_node.in_) {
    // skip [ordering_node] -> [next_ordering_node] edge
    // TODO(julian) reorganize the ordering_edges so that we don't have to skip
    if (in_id == ordering_node.id_) {
      continue;
    }

    auto const& incoming_ordering_node = og.nodes_[in_id];

    auto const route_eotd = get_route_eotd(incoming_ordering_node.ir_id_,
                                           next_ordering_node.ir_id_, infra);

    if (!route_eotd && !incoming_ordering_node.has_next(og)) {
      // we don't have a route eotd in the incoming_ordering_node
      // but [next_incoming_ordering_node] does not exist, where we would
      // look for the signal eotd. thus, skipping.
      // TODO(julian) check if this is correct
      continue;
    }

    auto const& next_incoming_ordering_node = incoming_ordering_node.next(og);

    // if we have a fitting route eotd, get it from [incoming_ordering_node],
    // otherwise get the signal eotd from [next_incoming_ordering_node]

    auto const dependency =
        route_eotd
            ? find_element_in_interlocking_group(
                  *route_eotd,
                  sg.interlocking_groups_[incoming_ordering_node.id_], sg)
            : find_signal_eotd_in_interlocking_group(
                  sg.interlocking_groups_[next_incoming_ordering_node.id_], sg,
                  infra);

    dependencies.push_back(dependency);
  }

  return dependencies;
}

bool requires_dependency(simulation_graph::node const& s_node,
                         infrastructure const& infra) {
  auto const& element = infra->graph_.elements_[s_node.element_id_];
  return element->is(type::MAIN_SIGNAL) || element->is(type::APPROACH_SIGNAL);
}

void construct_edges(simulation_graph* sg, infrastructure const& infra,
                     timetable const&, ordering_graph const& og) {
  for (auto const& i_group : sg->interlocking_groups_) {
    for (auto const& s_node : i_group.skip_entry_ms(*sg)) {
      sg->train_dependencies_.emplace_back(
          requires_dependency(s_node, infra)
              ? get_train_dependencies(s_node, i_group, *sg, infra, og)
              : std::vector<simulation_graph::train_dependency>{});
    }
  }
}

void construct_nodes_for_trip(
    simulation_graph* sg, train::trip const& trip,
    std::pair<ordering_node::id, ordering_node::id> const& trip_nodes,
    infrastructure const& infra, timetable const& tt) {
  static const type_set simulation_node_types = {
      type::MAIN_SIGNAL, type::APPROACH_SIGNAL, type::EOTD, type::HALT};

  auto const& train = tt->trains_[trip.train_id_];

  auto ordering_node_id = trip_nodes.first;

  auto interlocking_group_start = sg->nodes_.size();

  simulation_graph::interlocking_group::offset interlocking_group_offset = 0;

  soro::optional<simulation_graph::interlocking_group::offset> approach_offset =
      std::nullopt;
  soro::optional<simulation_graph::interlocking_group::offset> halt_offset =
      std::nullopt;

  bool found_signal_eotd = false;

  auto const finish_interlocking_group = [&] {
    sg->interlocking_groups_.emplace_back(interlocking_group_start,
                                          sg->nodes_.size(), approach_offset,
                                          halt_offset);
    interlocking_group_start = sg->nodes_.size() - 1;
    interlocking_group_offset = 0;

    found_signal_eotd = false;
    ++ordering_node_id;

    halt_offset = std::nullopt;
    approach_offset = std::nullopt;
  };

  for (auto const& tn : train.iterate(infra)) {
    if (tn.omitted() || !simulation_node_types.contains(tn.node_->type())) {
      continue;
    }

    sg->timetable_dependencies_.emplace_back(tn.sequence_point_);
    sg->nodes_.emplace_back(tn.node_->element_->id());
    sg->simulation_node_to_ordering_node_.emplace_back(ordering_node_id);

    switch (tn.node_->type()) {
      case type::MAIN_SIGNAL: {
        finish_interlocking_group();
        break;
      }

      case type::EOTD: {
        auto const& eotd = infra->graph_.element_data<struct eotd>(tn.node_);
        if (eotd.signal_) {
          found_signal_eotd = true;
        } else if (!found_signal_eotd) {
          continue;
        }
        break;
      }

      case type::APPROACH_SIGNAL: {
        approach_offset = decltype(approach_offset){interlocking_group_offset};
        break;
      }

      case type::HALT: {
        utls::sassert(!halt_offset.has_value(), "double halt");
        halt_offset = decltype(halt_offset){interlocking_group_offset};
        break;
      }

      default: break;
    }

    ++interlocking_group_offset;
  }

  finish_interlocking_group();

  utls::ensure(ordering_node_id == trip_nodes.second);
  utls::ensure(sg->interlocking_groups_.back().to_ == sg->nodes_.size());
}

void construct_nodes(simulation_graph* sg, infrastructure const& infra,
                     timetable const& tt, ordering_graph const& og) {
  for (auto const& [trip, trip_nodes] : og.trip_to_nodes_) {
    auto const trip_start = sg->nodes_.size();

    construct_nodes_for_trip(sg, trip, trip_nodes, infra, tt);

    auto const trip_end = sg->nodes_.size();
    sg->trips_.emplace_back(trip_start, trip_end);
  }

  utls::ensures([&] {
    // check that all interlocking groups are valid
    for (auto const& [ig1, ig2] : utl::pairwise(sg->interlocking_groups_)) {
      utls::ensure(ig1.from_ < ig1.to_);
      utls::ensure(ig2.from_ < ig2.to_);

      utls::ensure(ig1.to_ == ig2.from_ || ig1.to_ == ig2.from_ + 1);
    }
  });
}

simulation_graph::simulation_graph(infrastructure const& infra,
                                   timetable const& timetable,
                                   ordering_graph const& og) {
  construct_nodes(this, infra, timetable, og);

  utls::ensure(
      interlocking_groups_.size() == og.nodes_.size(),
      "every interlocking group corresponds to a node in the ordering graph");

  construct_edges(this, infra, timetable, og);

  utls::ensures([this, &og] {
    std::size_t ordering_edges = 0;

    for (auto const& node : og.nodes_) {
      ordering_edges += node.out_.size() - static_cast<std::size_t>(
                                               og.next_[node.id_].has_value());
    }

    utls::ensure(train_dependencies_.size() == nodes_.size(),
                 "same amount of train dependencies as nodes");
  });
}

}  // namespace soro::simulation
