#include "soro/simulation/graph/graph.h"

#include <cstddef>
#include <algorithm>
#include <iterator>
#include <optional>
#include <span>
#include <vector>

#include "utl/concat.h"
#include "utl/logging.h"
#include "utl/pairwise.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/contains.h"
#include "soro/utls/std_wrapper/count_if.h"
#include "soro/utls/std_wrapper/find_if.h"
#include "soro/utls/std_wrapper/find_if_position.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/predicates.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/ordering/graph.h"

namespace soro::sim {

using namespace soro::tt;
using namespace soro::infra;

graph::node::id graph::node::invalid() { return id::invalid(); }

graph::node::id graph::node::get_id(graph const& g) const {
  utls::expect(!g.nodes_.empty(), "simulation graph has no nodes");
  utls::expect(this >= g.nodes_.data(), "node is not in simulation graph");
  utls::expect(this <= g.nodes_.data() + g.nodes_.size() - 1);

  return id{utls::narrow<id::value_t>(this - g.nodes_.data())};
}

graph::interlocking_group const& graph::node::get_interlocking_group(
    graph const& g) const {
  return g.interlocking_groups_[g.simulation_node_to_ordering_node_[get_id(g)]];
}

graph::trip_group const& graph::node::get_trip_group(graph const& g) const {
  return g.trips_[g.node_to_trip_group_[get_id(g)]];
}

graph::simulation_group const& graph::node::get_simulation_group(
    graph const& g) const {
  return g.simulation_groups_[g.node_to_simulation_group_[get_id(g)]];
}

bool graph::node::has_train_dependencies(graph const& g) const {
  return !in(g).empty();
}

graph::train_dependencies_t::const_bucket graph::node::in(
    graph const& g) const {
  return g.train_dependencies_[get_id(g)];
}

bool graph::node::has_train_dependents(graph const& g) const {
  return !out(g).empty();
}

graph::train_dependents_t::const_bucket graph::node::out(graph const& g) const {
  return g.train_dependents_[get_id(g)];
}

bool graph::node::is_first_in_trip(graph const& g) const {
  return get_trip_group(g).from_ == get_id(g);
}

bool graph::node::is_last_in_trip(graph const& g) const {
  return get_trip_group(g).to_ - 1 == get_id(g);
}

bool graph::node::can_have_train_dependency(infrastructure const& infra) const {
  auto const& element = infra->graph_.elements_[element_id_];
  return element->is_any({type::MAIN_SIGNAL, type::APPROACH_SIGNAL});
}

graph::node const& graph::group::front(graph const& g) const {
  return g.nodes_[from_];
}

graph::node const& graph::group::back(graph const& g) const {
  return g.nodes_[to_ - 1];
}

graph::node const& graph::group::at(graph const& g,
                                    graph::group::offset const idx) const {
  utls::sassert(idx < as_val(to_ - from_), "idx {} not in range", idx);
  return g.nodes_[from_ + idx];
}

std::span<graph::node const> graph::group::nodes(graph const& g) const {
  return {std::begin(g.nodes_) + as_val(from_),
          std::begin(g.nodes_) + as_val(to_)};
}

graph::group::offset graph::group::size() const {
  return utls::narrow<offset>(as_val(to_ - from_));
}

bool graph::group::empty() const { return to_ == from_; }

graph::interlocking_group::interlocking_group(
    graph::node::id const from, graph::node::id const to,
    graph::group::opt_offset const approach_signal,
    graph::group::opt_offset const halt)
    : group{from, to}, last_halt_{halt}, approach_signal_{approach_signal} {}

bool graph::interlocking_group::has_halt() const {
  return last_halt_.has_value();
}

bool graph::interlocking_group::has_approach() const {
  return approach_signal_.has_value();
}

std::span<graph::node const> graph::interlocking_group::skip_entry_ms(
    graph const& g) const {
  // the first node in a trip group is a halt, so don't skip it
  // every other first node in an interlocking group is a ms
  auto const skip = front(g).get_trip_group(g).from_ != from_;

  return {std::begin(g.nodes_) + as_val(from_) + (skip ? 1 : 0),
          std::begin(g.nodes_) + as_val(to_)};
}

bool graph::interlocking_group::approach_before_halt() const {
  utls::expect(has_halt(), "approach_before_halt called on group without halt");
  utls::expect(has_approach(),
               "approach_before_halt called on group without approach");

  return *last_halt_ > *approach_signal_;
}

graph::trip_group::trip_group(node::id const from, node::id const to,
                              train::trip::id const trip_id,
                              train::id const train_id,
                              absolute_time const anchor)
    : group{from, to}, train::trip{trip_id, train_id, anchor} {}

graph::trip_group::id graph::trip_group::get_id(graph const& g) const {
  utls::expect(!g.trips_.empty(), "simulation graph has no trips");
  utls::expect(this >= g.trips_.data(), "trip is not in simulation graph");
  utls::expect(this <= g.trips_.data() + g.trips_.size() - 1);

  return id{utls::narrow<id::value_t>(this - g.trips_.data())};
}

graph::simulation_group::simulation_group(node::id const from,
                                          node::id const to)
    : group{from, to} {}

graph::simulation_group::id graph::simulation_group::get_id(
    graph const& g) const {
  utls::expect(!g.simulation_groups_.empty(), "no simulation groups");
  utls::expect(this >= g.simulation_groups_.data(), "this not in range");
  utls::expect(
      this <= g.simulation_groups_.data() + g.simulation_groups_.size() - 1,
      "this not in range");

  return id{utls::narrow<id::value_t>(this - g.simulation_groups_.data())};
}

graph::trip_group::id const& graph::simulation_group::get_trip_id(
    graph const& g) const {
  utls::expect(!this->empty(), "got empty simulation group");
  auto const& trip_id = g.node_to_trip_group_[this->front(g).get_id(g)];
  utls::sassert(trip_id < g.trips_.size(), "out of bounds access");
  return trip_id;
}

graph::trip_group const& graph::simulation_group::get_trip(
    graph const& g) const {
  return g.trips_[get_trip_id(g)];
}

bool graph::simulation_group::has_previous(graph const& g) const {
  return !front(g).is_first_in_trip(g);
}

graph::simulation_group::id graph::simulation_group::previous_id(
    graph const& g) const {
  utls::expect(has_previous(g), "no previous for {}", get_id(g));
  return get_id(g) - 1;
}

graph::simulation_group const& graph::simulation_group::previous(
    graph const& g) const {
  utls::expect(has_previous(g), "no previous for {}", get_id(g));
  return g.simulation_groups_[previous_id(g)];
}

bool graph::simulation_group::has_next(graph const& g) const {
  return !back(g).is_last_in_trip(g);
}

graph::simulation_group::id graph::simulation_group::next_id(
    graph const& g) const {
  utls::expect(has_next(g), "no next for {}", get_id(g));
  return get_id(g) + 1;
}

graph::simulation_group const& graph::simulation_group::next(
    graph const& g) const {
  utls::expect(has_next(g), "no next for {}", get_id(g));
  return g.simulation_groups_[next_id(g)];
}

bool graph::simulation_group::has_train_dependencies(graph const& g) const {
  return !get_train_dependencies(g).empty();
}

graph::simulation_group::train_dependencies_t::const_bucket
graph::simulation_group::get_train_dependencies(graph const& g) const {
  return g.simulation_group_dependencies_[get_id(g)];
}

bool graph::simulation_group::has_train_dependents(graph const& g) const {
  return !get_train_dependents(g).empty();
}

graph::simulation_group::train_dependents_t::const_bucket
graph::simulation_group::get_train_dependents(graph const& g) const {
  return g.simulation_group_dependents_[get_id(g)];
}

infra::type_set graph::occuring_types() {
  return infra::type_set{infra::type::APPROACH_SIGNAL, infra::type::MAIN_SIGNAL,
                         infra::type::EOTD, infra::type::HALT};
}

graph::train_dependencies_t::const_bucket graph::in(node::id const id) const {
  return nodes_[id].in(*this);
}

graph::train_dependents_t::const_bucket graph::out(node::id const id) const {
  return nodes_[id].out(*this);
}

soro::optional<graph::node::id> find_element_in_interlocking_group(
    element::id const e, graph::interlocking_group const& interlocking_group,
    graph const& sg, infrastructure const& infra) {
  auto const it =
      utls::find_if(interlocking_group.skip_entry_ms(sg),
                    [&](auto&& node) { return node.element_id_ == e; });

  if (it == std::end(interlocking_group.skip_entry_ms(sg))) {
    utls::sasserts([&] {
      auto const has_halt = interlocking_group.has_halt();
      auto const starts_with_halt =
          has_halt &&
          infra->graph_.elements_[interlocking_group.front(sg).element_id_]->is(
              type::HALT);
      auto const ends_with_halt =
          has_halt &&
          infra->graph_.elements_[interlocking_group.back(sg).element_id_]->is(
              type::HALT);
      utls::sassert(starts_with_halt || ends_with_halt);
    });
    return std::nullopt;
  } else {
    utls::sassert(it != std::end(interlocking_group.skip_entry_ms(sg)));
    return soro::optional<graph::node::id>{it->get_id(sg)};
  }
}

graph::node::id find_next_signal_eotd_or_last_halt(
    graph::node::id const from, graph const& sg, infrastructure const& infra) {
  auto const to = sg.nodes_[from].get_trip_group(sg).to_;

  utls::sassert(from < to, "from {} is larger than to {}", from, to);

  auto const search_space = std::span{std::begin(sg.nodes_) + as_val(from),
                                      std::begin(sg.nodes_) + as_val(to)};

  auto const it = utls::find_if(search_space, [&infra](auto&& node) {
    auto const& element = infra->graph_.elements_[node.element_id_];
    return element->is(type::EOTD);
  });

  if (it == std::end(search_space)) {
    return search_space.back().get_id(sg);
  } else {
    utls::sassert(is_signal_eotd(it->element_id_, infra));
    return it->get_id(sg);
  }
}

soro::optional<element::id> get_route_eotd(interlocking_route const& from,
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

  auto last_reotd = element::invalid();
  for (auto idx = static_cast<std::ptrdiff_t>(cps[from.id_].size()) - 1;
       idx >= 0; --idx) {

    auto const& cp = cps[from.id_][utls::narrow<soro::size_t>(idx)];

    if (cp.type_ == type::EOTD) {
      last_reotd = cp.element_;
      continue;
    }

    auto const to_contains = utls::contains(
        cps[to.id_], cps[from.id_][utls::narrow<soro::size_t>(idx)]);

    if (to_contains) {
      return last_reotd == element::invalid()
                 ? soro::optional<element::id>{std::nullopt}
                 : soro::optional<element::id>{last_reotd};
    }
  }

  return std::nullopt;
}

soro::optional<element::id> get_route_eotd(interlocking_route::id const from,
                                           interlocking_route::id const to,
                                           infrastructure const& infra) {
  return get_route_eotd(infra->interlocking_.routes_[from],
                        infra->interlocking_.routes_[to], infra);
}

soro::vector<graph::train_dependency> get_train_dependencies(
    graph::node const& s_node, graph::interlocking_group const& ig,
    graph const& sg, infrastructure const& infra, ordering::graph const& og) {

  auto const& element = infra->graph_.elements_[s_node.element_id_];

  utls::sassert(
      element->is(type::MAIN_SIGNAL) || element->is(type::APPROACH_SIGNAL),
      "train dependency on {}", get_type_str(element->type()));

  auto const as = element->is(type::APPROACH_SIGNAL);
  auto const ms = element->is(type::MAIN_SIGNAL);

  // when the approach signal that signals for the next interlocking route
  // is located before a halt, we don't have to check for its dependencies,
  // as we are going to stop anyway, whether it signals stop or go
  auto const as_before_halt = as && ig.has_halt() && ig.approach_before_halt();

  // main signals without any halts in the interlocking route don't have any
  // dependencies. the approach signals have them instead
  auto const ms_without_halt = ms && !ig.has_halt();

  // if the main signal is located after the halt, but the approach signal is
  // too, we put the dependencies on the approach signal
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
   * the element from s_node that will receive the dependency is located in
   * the interlocking route given by ordering_node
   */

  soro::vector<graph::train_dependency> dependencies;

  auto const& ordering_node =
      og.nodes_[sg.simulation_node_to_ordering_node_[s_node.get_id(sg)]];

  // last interlocking route in the train run, we don't have any dependencies
  // anymore
  if (!ordering_node.has_next(og)) {
    return {};
  }

  auto const& next_ordering_node = ordering_node.next(og);

  for (auto const in_id : next_ordering_node.in(og)) {
    // skip [ordering_node] -> [next_ordering_node] edge
    // TODO(julian) reorganize the ordering_edges so that we don't have to skip
    if (in_id == ordering_node.get_id(og)) {
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

    // if we have a fitting route eotd, get it from [incoming_ordering_node],
    if (route_eotd) {
      auto const dependency = find_element_in_interlocking_group(
          *route_eotd,
          sg.interlocking_groups_[incoming_ordering_node.get_id(og)], sg,
          infra);

      if (dependency.has_value()) {
        dependencies.push_back(*dependency);
      }

    } else {
      // otherwise get the signal eotd from [next_incoming_ordering_node]
      auto const from =
          sg.interlocking_groups_[incoming_ordering_node.next_id(og)].from_;
      auto const dependency =
          find_next_signal_eotd_or_last_halt(from, sg, infra);
      dependencies.push_back(dependency);
    }
  }

  return dependencies;
}

graph::train_dependents_t get_train_dependents(
    graph::train_dependencies_t const& train_dependencies) {
  utls::expect(!train_dependencies.empty(), "train_dependencies are empty");

  utl::scoped_timer const timer("creating train dependents");

  // as train dependencies are incoming edges for the simulation groups
  // we will produce the train dependents, i.e. the outgoing edges

  // three pass approach

  // 1. count how many outgoing edges every node has
  soro::vector_map<graph::node::id, soro::size_t> outgoing_edges_count(
      train_dependencies.size());
  for (auto const dependencies : train_dependencies) {
    for (auto const dependency : dependencies) {
      ++outgoing_edges_count[dependency];
    }
  }

  // 2. depending on the outgoing edges count allocate the memory in the result
  graph::train_dependents_t result;
  result.data_.reserve(train_dependencies.data_.size());
  result.bucket_starts_.reserve(train_dependencies.bucket_starts_.size());
  for (auto out_count : outgoing_edges_count) {
    // TODO(julian) stack vector would be nice here ...
    result.emplace_back(
        std::vector<graph::train_dependent>(out_count, graph::node::invalid()));
  }

  // 3. fill the result with the outgoing edges read from the incoming edges
  // use current_fill as a helper, to notify the bucket fill count, as we
  // allocated them all in step 2
  soro::vector_map<graph::node::id, soro::size_t> current_fill(
      train_dependencies.size(), 0);
  for (auto to = graph::node::id{0}; to < train_dependencies.size(); ++to) {
    for (auto const from : train_dependencies[to]) {
      result[from][current_fill[from]] = to;
      ++current_fill[from];
    }
  }

  // bonus. make sure it's correct
  utls::ensures([&] {
    utls::ensure(result.size() == train_dependencies.size());

    // result must contain every edge from sg.train_dependencies_
    for (auto to = graph::node::id{0}; to < train_dependencies.size(); ++to) {
      for (auto const from : train_dependencies[to]) {
        utls::ensure(utls::contains(result[from], to));
      }
    }
  });

  return result;
}

graph::train_dependencies_t get_train_dependencies(
    graph const& sg, ordering::graph const& og, infrastructure const& infra) {
  utl::scoped_timer const timer("determining train dependencies");

  graph::train_dependencies_t result;

  for (auto const& i_group : sg.interlocking_groups_) {
    for (auto const& s_node : i_group.skip_entry_ms(sg)) {
      result.emplace_back(
          s_node.can_have_train_dependency(infra)
              ? get_train_dependencies(s_node, i_group, sg, infra, og)
              : soro::vector<graph::train_dependency>{});
    }
  }

  return result;
}

struct trip_nodes_result {
  soro::vector_map<graph::node::id, graph::node> nodes_;
  soro::vector_map<graph::node::id, graph::timetable_dependency>
      timetable_dependencies_;
  soro::vector_map<graph::node::id, ordering::graph::node::id>
      node_to_ordering_node_;
  soro::vector_map<ordering::graph::node::id, graph::interlocking_group>
      interlocking_groups_;
};

trip_nodes_result get_trip_nodes(train const& train,
                                 ordering::graph::trip_group const& trip,
                                 infrastructure const& infra) {
  trip_nodes_result result;

  auto ordering_node_id = trip.from_;

  graph::group::offset interlocking_group_offset{0};

  graph::node::id interlocking_group_start{0};

  graph::group::opt_offset approach_offset{std::nullopt};
  graph::group::opt_offset halt_offset{std::nullopt};

  bool got_signal_eotd = false;

  auto const should_skip = [&infra](train_node const& train_node,
                                    bool const got_signal_eotd_inner) {
    // skip if the node/element is omitted anyway
    // e.g. omitted speed limit or halt without sequence point
    if (train_node.omitted()) return true;

    // skip if we do not care about the type in the simulation graph
    static const infra::type_set node_types = {
        infra::type::TRACK_END,   infra::type::BUMPER,
        infra::type::MAIN_SIGNAL, infra::type::APPROACH_SIGNAL,
        infra::type::EOTD,        infra::type::HALT};

    auto const skip_due_to_type =
        !node_types.contains(train_node.node_->type());

    if (skip_due_to_type) return true;

    // skip if it is a signal eotd after we already found one
    auto const signal_eotd = train_node.node_->is(type::EOTD) &&
                             is_signal_eotd(train_node.node_, infra);
    auto const first_signal_eotd = !got_signal_eotd_inner;

    if (signal_eotd && !first_signal_eotd) return true;

    // skip if it is a route eotd before we found a signal eotd
    auto const route_eotd = train_node.node_->is(type::EOTD) &&
                            is_route_eotd(train_node.node_, infra);
    auto const before_signal_eotd = !got_signal_eotd_inner;

    if (route_eotd && before_signal_eotd) return true;

    return false;
  };

  for (auto const& tn : train.iterate(infra)) {
    if (should_skip(tn, got_signal_eotd)) continue;

    result.nodes_.emplace_back(tn.node_->element_->get_id());
    result.timetable_dependencies_.emplace_back(tn.sequence_point_);
    result.node_to_ordering_node_.emplace_back(ordering_node_id);

    switch (tn.node_->type()) {
      case type::MAIN_SIGNAL: {

        // add the interlocking group with the current state
        result.interlocking_groups_.emplace_back(
            interlocking_group_start, graph::node::id{result.nodes_.size()},
            approach_offset, halt_offset);

        // reset the state for the interlocking groups as we have finished one
        interlocking_group_offset = 0;

        interlocking_group_start = graph::node::id{
            utls::narrow<graph::node::id::value_t>(result.nodes_.size() - 1)};

        got_signal_eotd = false;

        halt_offset.reset();
        approach_offset.reset();

        // increase the ordering node id as we have reached the end of an
        // interlocking route. thus, we move over to the next node
        ++ordering_node_id;

        break;
      }

      case type::EOTD: {
        got_signal_eotd |= is_signal_eotd(tn.node_, infra);
        break;
      }

      case type::APPROACH_SIGNAL: {
        approach_offset.emplace(interlocking_group_offset);
        break;
      }

      case type::HALT: {
        halt_offset.emplace(interlocking_group_offset);
        break;
      }

      default: break;
    }

    ++interlocking_group_offset;
  }

  // add the interlocking group with the current state
  result.interlocking_groups_.emplace_back(
      interlocking_group_start, graph::node::id{result.nodes_.size()},
      approach_offset, halt_offset);

  utls::ensure(ordering_node_id + 1 == trip.to_);
  utls::ensure(result.interlocking_groups_.back().to_ == result.nodes_.size());

  utls::ensure(result.nodes_.size() == result.node_to_ordering_node_.size());
  utls::ensure(result.nodes_.size() == result.timetable_dependencies_.size());

  return result;
}

void construct_nodes(graph* sg, infrastructure const& infra,
                     timetable const& tt, ordering::graph const& og) {
  utl::scoped_timer const timer("constructing simulation graph nodes");

  for (auto const& trip : og.trips_) {
    auto const trip_start = graph::node::id{sg->nodes_.size()};

    auto const& train = tt->trains_[trip.train_id_];
    auto sim_nodes = get_trip_nodes(train, trip, infra);

    for (auto& interlocking_group : sim_nodes.interlocking_groups_) {
      interlocking_group.from_ += sg->nodes_.size();
      interlocking_group.to_ += sg->nodes_.size();
    }

    utl::concat(sg->nodes_, sim_nodes.nodes_);
    utl::concat(sg->timetable_dependencies_, sim_nodes.timetable_dependencies_);
    utl::concat(sg->simulation_node_to_ordering_node_,
                sim_nodes.node_to_ordering_node_);
    utl::concat(sg->interlocking_groups_, sim_nodes.interlocking_groups_);

    auto const trip_end = graph::node::id{sg->nodes_.size()};

    soro::vector_map<graph::node::id, graph::trip_group::id> node_to_trip_group;
    graph::trip_group::id const trip_id{sg->trips_.size()};
    node_to_trip_group.resize(as_val(trip_end - trip_start), trip_id);
    utl::concat(sg->node_to_trip_group_, node_to_trip_group);

    sg->trips_.emplace_back(trip_start, trip_end, trip_id, trip.train_id_,
                            trip.anchor_);
  }

  utls::ensures([&] {
    // check that all interlocking groups are valid
    for (auto const [ig1, ig2] : utl::pairwise(sg->interlocking_groups_)) {
      utls::ensure(ig1.from_ < ig1.to_);
      utls::ensure(ig2.from_ < ig2.to_);

      utls::ensure(ig1.to_ == ig2.from_ || ig1.to_ == ig2.from_ + 1);
    }
  });
}

graph::simulation_groups_t get_simulation_groups(graph const& sg) {
  utl::scoped_timer const timer("generating simulation groups");

  graph::simulation_groups_t result;

  auto const starts_simulation_group = [&sg](auto&& sim_node) {
    return sim_node.has_train_dependencies(sg);
  };

  for (auto const& trip : sg.trips_) {
    graph::node::id from = trip.from_;
    graph::node::id to = trip.from_;

    while (to != trip.to_) {
      to += utls::find_if_position<soro::size_t>(
                std::begin(sg.nodes_) + as_val(from) + 1,
                std::begin(sg.nodes_) + as_val(trip.to_),
                starts_simulation_group) +
            1;
      result.emplace_back(from, to + (to != trip.to_ ? 1 : 0));
      from = to;
    }
  }

  utls::ensures([&] {
    auto const check_sim_group_node = [&](graph::node const& n) {
      auto const has_train_dependency = n.has_train_dependencies(sg);

      auto const node_id = n.get_id(sg);
      auto const& trip = n.get_trip_group(sg);

      auto const is_first_in_trip = node_id == trip.front(sg).get_id(sg);
      auto const is_last_in_trip = node_id == trip.back(sg).get_id(sg);

      return has_train_dependency || is_first_in_trip || is_last_in_trip;
    };

    std::vector<bool> cover(sg.nodes_.size(), false);

    for (auto const& sim_group : result) {
      utls::ensure(check_sim_group_node(sim_group.front(sg)),
                   "sim group start incorrect");
      utls::ensure(check_sim_group_node(sim_group.back(sg)),
                   "sim group end incorrect");

      std::fill(std::begin(cover) + as_val(sim_group.from_),
                std::begin(cover) + as_val(sim_group.to_), true);
    }

    utls::ensure(utls::all_of(cover), "sim groups don't cover all nodes");
  });

  return result;
}

graph::simulation_group::train_dependencies_t get_simulation_group_dependencies(
    graph const& g) {
  utl::scoped_timer const timer("creating simulation group dependencies");

  graph::simulation_group::train_dependencies_t result;

  for (auto const& sg : g.simulation_groups_) {
    auto deps = soro::to_vec(sg.front(g).in(g), [&](auto&& n_id) {
      return g.nodes_[n_id].get_simulation_group(g).get_id(g);
    });

    if (sg.has_previous(g)) {
      deps.emplace_back(sg.previous_id(g));
    }

    result.emplace_back(deps);
  }

  utls::ensure(result.size() == g.simulation_groups_.size(),
               "wrong size for simulation group mapping");

  return result;
}

graph::simulation_group::train_dependents_t get_simulation_group_dependents(
    graph::simulation_group::train_dependencies_t const& sg_dependencies) {
  utl::scoped_timer const timer("creating simulation group dependents");

  utls::expect(!sg_dependencies.empty(), "train_dependencies are empty");

  // as train dependencies are incoming edges for the simulation groups
  // we will produce the train dependents, i.e. the outgoing edges

  // three pass approach

  // 1. count how many outgoing edges every node has
  std::vector<soro::size_t> outgoing_edges_count(sg_dependencies.size());
  for (auto const dependencies : sg_dependencies) {
    for (auto const dependency : dependencies) {
      ++outgoing_edges_count[dependency.v_];
    }
  }

  // 2. depending on the outgoing edges count allocate the memory in the result
  graph::simulation_group::train_dependents_t result;
  result.data_.reserve(sg_dependencies.data_.size());
  result.bucket_starts_.reserve(sg_dependencies.bucket_starts_.size());
  for (auto out_count : outgoing_edges_count) {
    // TODO(julian) stack vector would be nice here ...
    result.emplace_back(std::vector<graph::simulation_group::id>(
        out_count, graph::simulation_group::invalid()));
  }

  // 3. fill the result with the outgoing edges read from the incoming edges
  // use current_fill as a helper, to notify the bucket fill count, as we
  // allocated them all in step 2
  std::vector<soro::size_t> current_fill(sg_dependencies.size(), 0);
  for (graph::simulation_group::id to{0}; to < sg_dependencies.size(); ++to) {
    for (auto const from : sg_dependencies[to]) {
      result[from][current_fill[from.v_]] = to;
      ++current_fill[from.v_];
    }
  }

  // bonus. make sure it's correct
  utls::ensures([&] {
    utls::ensure(result.size() == sg_dependencies.size(),
                 "wrong size for simulation group mapping");

    // result must contain every edge from sg.train_dependencies_
    for (graph::simulation_group::id to{0}; to < sg_dependencies.size(); ++to) {
      for (auto const from : sg_dependencies[to]) {
        utls::ensure(utls::contains(result[from], to));
      }
    }
  });

  return result;
}

auto get_node_to_simulation_group(graph const& g) {
  utl::scoped_timer const timer("creating node to simulation group mapping");

  soro::vector_map<graph::node::id, graph::simulation_group::id> result(
      g.nodes_.size(), graph::simulation_group::invalid());

  for (auto const& sg : g.simulation_groups_) {
    std::fill(std::begin(result) + as_val(sg.from_),
              std::begin(result) + as_val(sg.to_), sg.get_id(g));
  }

  utls::ensure(result.size() == g.nodes_.size(),
               "wrong size for a node mapping");
  utls::ensure(utls::all_of(result, [](auto&& id) {
    return id != graph::simulation_group::invalid();
  }));

  return result;
}

void print_stats(graph const& g) {

  uLOG(utl::info) << "simulation graph nodes: " << g.nodes_.size();
  uLOG(utl::info) << "simulation graph simulation groups: "
                  << g.simulation_groups_.size();
  uLOG(utl::info) << "simulation graph train dependencies: "
                  << g.train_dependencies_.data_.size();

  auto const tt_deps = utls::count_if(
      g.timetable_dependencies_, [](auto&& dep) { return dep.has_value(); });
  uLOG(utl::info) << "simulation graph timetable dependencies: " << tt_deps;
}

graph::graph(infrastructure const& infra, timetable const& timetable,
             ordering::graph const& og) {
  utl::scoped_timer const timer("creating simulation graph");

  construct_nodes(this, infra, timetable, og);

  utls::ensure(
      interlocking_groups_.size() == og.nodes_.size(),
      "every interlocking group corresponds to a node in the ordering graph");
  utls::ensure(simulation_node_to_ordering_node_.size() == nodes_.size(),
               "incorrect size for sim node to ordering node mapping");
  utls::ensure(timetable_dependencies_.size() == nodes_.size(),
               "same amount of allocated timetable dependencies as nodes");
  utls::ensure(node_to_trip_group_.size() == nodes_.size(),
               "incorrect size for sim node to trip group");

  train_dependencies_ = get_train_dependencies(*this, og, infra);
  train_dependents_ = get_train_dependents(train_dependencies_);

  utls::ensure(train_dependencies_.size() == nodes_.size(),
               "same amount of train dependencies as nodes");

  simulation_groups_ = get_simulation_groups(*this);
  node_to_simulation_group_ = get_node_to_simulation_group(*this);
  simulation_group_dependencies_ = get_simulation_group_dependencies(*this);
  simulation_group_dependents_ =
      get_simulation_group_dependents(simulation_group_dependencies_);

  print_stats(*this);

  utls::ensures([this] {
    auto const non_empty_buckets = utls::count_if(
        train_dependencies_, [](auto&& bucket) { return !bucket.empty(); });

    utls::ensure(simulation_groups_.size() == non_empty_buckets + trips_.size(),
                 "every train dependency concludes a simulation group, as does "
                 "every trip "
                 "end");
  });
}

}  // namespace soro::sim
