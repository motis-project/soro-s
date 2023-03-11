#include "soro/simulation/ordering_graph.h"

#include "utl/enumerate.h"

#include "soro/utls/container/priority_queue.h"
#include "soro/utls/std_wrapper/std_wrapper.h"

#include <algorithm>
#include <random>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "soro/runtime/runtime.h"

namespace soro::simulation {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;
using namespace std;
using namespace rapidjson;

struct route_usage {
  utls::unixtime from_{utls::INVALID_TIME};
  utls::unixtime to_{utls::INVALID_TIME};
  ordering_node::id node_id_{ordering_node::INVALID};
};

template <typename Writer>
void ordering_node::serialize(Writer& writer) {
  writer.StartArray();
  writer.String(std::to_string(id_).c_str());

  writer.Uint(ir_id_);

  writer.Uint(train_id_);

  writer.EndArray();
}

using usage_idx = uint32_t;
constexpr auto const INVALID_USAGE_IDX = std::numeric_limits<usage_idx>::max();

ordering_graph::ordering_graph(infra::infrastructure const& infra,
                               tt::timetable const& tt) {
  utls::sassert(false, "Not implemented");

  std::vector<std::vector<route_usage>> route_orderings(
      infra->interlocking_.routes_.size());

  for (auto const& train : tt->trains_) {
    auto const stamps = runtime_calculation(train, infra, {type::MAIN_SIGNAL});

    utl::verify(train.path_.size() == stamps.times_.size() + 1,
                "Differing amounts of signal station routes in train path and "
                "main signals in running time calculation timestamps");

    for (auto const [idx, ir_id] : utl::enumerate(train.path_)) {
      auto const id = static_cast<ordering_node::id>(nodes_.size());

      nodes_.emplace_back(id, ir_id, train.id_);

      utls::unixtime const from;
      utls::unixtime const to;

      route_orderings[ir_id].push_back(
          {.from_ = from, .to_ = to, .node_id_ = id});
    }
  }

  for (auto& usage_order : route_orderings) {
    utls::sort(usage_order, [](auto&& usage1, auto&& usage2) {
      return usage1.from_ < usage2.from_;
    });
  }

  // holds an index for every std::vector<route_usage> in route_orderings
  // the index points to the smallest not yet processed route_usage object
  std::vector<size_t> current_route_usage_index(route_orderings.size(),
                                                INVALID_USAGE_IDX);

  auto const entry_comparison = [&](auto const e1, auto const e2) {
    return route_orderings[e1][current_route_usage_index[e1]].from_ >
           route_orderings[e2][current_route_usage_index[e2]].from_;
  };

  // the priority queue contains every interlocking route that is used
  // by at least one train contained in the timetable.
  // the elements of the queue are sorted so that the top element is the
  // usage of an interlocking route with the smallest from_ time value of its
  // usage w.r.t to the current route usage index
  utls::priority_queue<ir_id, decltype(entry_comparison)> todo_queue(
      entry_comparison);

  for (auto const [route_id, usages] : utl::enumerate(route_orderings)) {
    if (usages.empty()) {
      continue;
    }

    current_route_usage_index[route_id] = 0;
    todo_queue.emplace(static_cast<ir_id>(route_id));
  }

  // the algorithm is similar to a merge operation
  // take the currently smallest route usage and add edges to all other
  // nodes with interlocking routes that are excluded
  // this is correct since we have taken the smallest (earliest) usage,
  // so all other nodes (i.e. usages) follow the currently processed usage
  // then increment the usage index of the interlocking route and re-add it
  // to the queue
  while (!todo_queue.empty()) {
    auto from_ssr = todo_queue.top();
    todo_queue.pop();

    auto& from_idx = current_route_usage_index[from_ssr];
    auto const from_usage = route_orderings[from_ssr][from_idx];
    auto const from_node = from_usage.node_id_;

    for (auto const& to_ir : infra->interlocking_.exclusions_[from_ssr]) {
      auto to_idx = current_route_usage_index[to_ir];

      if (to_idx == INVALID_USAGE_IDX) {
        continue;
      }

      // every interlocking route is in conflict with itself
      // since we are currently processing the usage of an interlocking
      // route we have to manually increment the usage index to get the next
      // usage of a different train of the same interlocking route
      if (to_ir == from_ssr) {
        if (to_idx < route_orderings[to_ir].size() - 1) {
          ++to_idx;
        } else {
          // if its the last usage of the interlocking route we can simply
          // continue, as we there will not be an edge
          continue;
        }
      }

      auto const& to_usage = route_orderings[to_ir][to_idx];
      auto const to_node = to_usage.node_id_;

      nodes_[from_node].out_.emplace_back(to_node);
      nodes_[to_node].in_.emplace_back(from_node);
    }

    if (from_idx < route_orderings[from_ssr].size() - 1) {
      ++from_idx;
      todo_queue.emplace(from_ssr);
    }
  }
}

ordering_graph::ordering_graph() = default;

/**
 * Generates a graph with given amount of trains, tracks and a given minimum and
 * maximum amount of nodes per train.
 *
 * Example: 2 trains with 5 tracks, and minimum of 2 and maximum of 5 could
 * yield a graph, where:
 * train A follows tracks 1, 2, 3, 4 and 5 (all 5 tracks)
 * train B follows tracks 3 and 1 (just 2 tracks)
 */
ordering_graph generate_testgraph(const int train_amnt, const int track_amnt,
                                  const int min_nodes, const int max_nodes) {
  std::random_device rd;
  return generate_testgraph(train_amnt, track_amnt, min_nodes, max_nodes, rd());
}

ordering_graph generate_testgraph(const int train_amnt, const int track_amnt,
                                  const int min_nodes, const int max_nodes,
                                  const unsigned int seed) {
  ordering_graph graph;

  // are min and max valid?
  const int max = max_nodes <= track_amnt ? max_nodes : track_amnt;
  const int min = min_nodes >= 0 ? min_nodes > max ? max : min_nodes : 0;

  // get random number generator
  std::mt19937 mt(seed);
  std::uniform_int_distribution<> distr_node(min, max);
  std::uniform_int_distribution<> distr_track(0, track_amnt - 1);

  // generate nodes for each train and connect them
  for (auto train = 0; train < train_amnt; train++) {
    // amount of tracks this train will use for now
    const auto node_amnt = distr_node(mt);
    std::vector<int> chosen_ids;

    for (auto n = 0; n < node_amnt; n++) {
      // choose a random new track that this train will use
      int track_id = 0;
      while (true) {
        track_id = distr_track(mt);
        // track must not already be used by this train
        if (!utls::contains(chosen_ids, track_id)) {
          chosen_ids.emplace_back(track_id);
          break;
        }
      }
      const auto size = graph.nodes_.size();
      graph.nodes_.emplace_back(static_cast<ordering_node::id>(size), track_id,
                                train);
      // connect the node before this one with the new one
      if (n != 0) {
        ordering_graph::emplace_edge(graph.nodes_[size - 1],
                                     graph.nodes_[size]);
      }
    }
  }

  // iterate over all nodes and test, whether future nodes have other trains on
  // the same track to generate edges (nodes further back will be trains with
  // higher ids!)
  const auto size = graph.nodes_.size();
  for (uint64_t first_index = 0; first_index < size; first_index++) {
    for (auto second_index = first_index + 1; second_index < size;
         second_index++) {
      // other train on different track is irrelevant and same
      // train can not have same track
      if (graph.nodes_[first_index].ir_id_ != graph.nodes_[second_index].ir_id_)
        continue;
      // connect first train with problematic second
      ordering_graph::emplace_edge(graph.nodes_[first_index],
                                   graph.nodes_[second_index]);
      // to prevent unnecessary edges (will be added transivitely): move
      // firstIndex further
      break;
    }
  }

  return graph;
}

ordering_node* ordering_graph::node_by_id(const ordering_node::id id) {
  auto node = find_if(nodes_.begin(), nodes_.end(),
                      [id](const ordering_node& n) { return n.id_ == id; });

  if (node == nodes_.end()) {
    return nullptr;
  } else {
    return &(*node);
  }
}

/**
 * Flips a single edge in the graph that orignates at from and end at to. It
 * does not propagate chages to parallel edges, but changes incoming/outgoing
 * edges as needed.
 */
bool ordering_graph::invert_single_edge(ordering_node& from,
                                        ordering_node& to) {
  if (!utls::contains(from.out_, to.id_)) {
    return false;
  }

  std::vector<ordering_node::id> from_in, from_out, to_in, to_out;

  // re-oranize the incoming and outgoing edges of to and from
  for (const ordering_node::id id : from.in_) {
    auto node = node_by_id(id);
    // edges ending in from that belong to another train need to be ending in to
    // after the inversion
    if (node == nullptr || node->train_id_ == from.train_id_) {
      from_in.emplace_back(id);
    } else {
      to_in.emplace_back(id);
      // the out_ vector of the other node in this edge also needs adjusting
      replace(node->out_.begin(), node->out_.end(), from.id_, to.id_);
    }
  }
  for (const ordering_node::id id : from.out_) {
    // filter out the edge to flip
    if (id != to.id_) {
      from_out.emplace_back(id);
    }
  }
  for (const ordering_node::id id : to.in_) {
    // filter out the edge to flip
    if (id != from.id_) {
      to_in.emplace_back(id);
    }
  }
  for (const ordering_node::id id : to.out_) {
    auto node = node_by_id(id);
    // edges originating in to that belong to another train need to be
    // originating in to after the inversion
    if (node == nullptr || node->train_id_ == to.train_id_) {
      to_out.emplace_back(id);
    } else {
      from_out.emplace_back(id);
      // the in_ vector of the other node in this edge also needs adjusting
      replace(node->in_.begin(), node->in_.end(), to.id_, from.id_);
    }
  }

  // re-insert the flip edge
  from_in.emplace_back(to.id_);
  to_out.emplace_back(from.id_);

  // finalization
  from.in_ = from_in;
  from.out_ = from_in;
  to.in_ = to_in;
  to.out_ = to_in;

  return true;
}

/**
 * Returns the first ordering_node in node.out_ which belongs to the same train
 * as node. Returns nullptr if no such node exists.
 */
ordering_node* ordering_graph::next_train_node(const ordering_node& node) {
  for (const ordering_node::id id : node.out_) {
    auto candidate = node_by_id(id);
    if (candidate != nullptr && node.train_id_ == candidate->train_id_) {
      return candidate;
    }
  }
  return nullptr;
}

/**
 * Returns the first ordering_node in node.in_ which belongs to the same train
 * as node. Returns nullptr if no such node exists.
 */
ordering_node* ordering_graph::prev_train_node(const ordering_node& node) {
  for (const ordering_node::id id : node.in_) {
    auto candidate = node_by_id(id);
    if (candidate != nullptr && node.train_id_ == candidate->train_id_) {
      return candidate;
    }
  }
  return nullptr;
}

/**
 * Flips the edge in the graph that orignates at from and end at to, propagating
 * changes to parallel edges that need to be flipped as well.
 */
void ordering_graph::invert_edge(ordering_node& from, ordering_node& to) {
  if (!invert_single_edge(from, to)) {
    // if there was no edge to flip, we also don't need to look for parallel
    // edges.
    return;
  }

  // walk the pointers forward along the train until there is no paralell edge
  // to flip
  auto from_ptr = next_train_node(*&from);
  auto to_ptr = next_train_node(*&to);
  while (from_ptr != nullptr && to_ptr != nullptr &&
         invert_single_edge(*from_ptr, *to_ptr)) {
    from_ptr = next_train_node(*from_ptr);
    to_ptr = next_train_node(*to_ptr);
  }

  // and walk back the pointers backwards along the train until there is no
  // parallel edge to flip
  from_ptr = prev_train_node(*&from);
  to_ptr = prev_train_node(*&to);

  while (from_ptr != nullptr && to_ptr != nullptr &&
         invert_single_edge(*from_ptr, *to_ptr)) {
    from_ptr = prev_train_node(*from_ptr);
    to_ptr = prev_train_node(*to_ptr);
  }
}

string ordering_graph::to_json() {
  StringBuffer sb;
  Writer<StringBuffer> writer(sb);
  serialize(writer);

  return sb.GetString();
}

void ordering_graph::emplace_edge(ordering_node& from, ordering_node& to) {
  from.out_.emplace_back(to.id_);
  to.in_.emplace_back(from.id_);
}

template <typename Writer>
void ordering_graph::serialize(Writer& writer) {
  writer.StartObject();

  writer.Key("a");
  writer.StartObject();
  writer.EndObject();

  writer.Key("n");
  writer.StartArray();
  for (ordering_node node : nodes_) {
    node.serialize(writer);
  }
  writer.EndArray();

  writer.Key("e");
  writer.StartArray();
  for (const ordering_node& node : nodes_) {
    for (const uint32_t to_id : node.out_) {
      writer.StartArray();

      writer.String(std::to_string(node.id_).c_str());

      writer.String(std::to_string(to_id).c_str());

      writer.EndArray();
    }
  }
  writer.EndArray();
  writer.EndObject();
}

ordering_graph from_json(const string& json) {
  ordering_graph graph;

  Document d;
  d.Parse(json.c_str());

  // Generate nodes
  for (const Value& node : d["n"].GetArray()) {
    graph.nodes_.emplace_back(static_cast<ordering_node::id>(
                                  std::stoul(node.GetArray()[0].GetString())),
                              node.GetArray()[1].GetUint(),
                              node.GetArray()[2].GetUint());
  }

  // Generate edges
  for (const Value& edge : d["e"].GetArray()) {
    ordering_graph::emplace_edge(
        graph.nodes_[std::stoul(edge.GetArray()[0].GetString())],
        graph.nodes_[std::stoul(edge.GetArray()[1].GetString())]);
  }

  return graph;
}

}  // namespace soro::simulation
