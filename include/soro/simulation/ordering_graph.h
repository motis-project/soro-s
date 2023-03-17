#pragma once

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"
#include "soro/utls/unixtime.h"

namespace soro::simulation {

using namespace rapidjson;

struct ordering_node {
  using id = std::uint32_t;
  static constexpr id INVALID = std::numeric_limits<id>::max();

  ordering_node(id const id, infra::interlocking_route::id const ir_id,
                tt::train::id const train_id)
      : id_{id}, ir_id_{ir_id}, train_id_{train_id} {}

  id id_{INVALID};
  infra::interlocking_route::id ir_id_;
  tt::train::id train_id_;

  std::vector<id> in_;
  std::vector<id> out_;

  template <typename Writer>
  void serialize(Writer& writer);
};

struct ordering_graph {
  ordering_graph(infra::infrastructure const& infra, tt::timetable const& tt);
  ordering_graph();
  std::vector<ordering_node> nodes_;
  string to_json();
  static void emplace_edge(ordering_node& from, ordering_node& to);

  string invert_edge(ordering_node& from, ordering_node& to);
  ordering_node* node_by_id(const ordering_node::id id);

  template <typename Writer>
  void serialize(Writer& node_writer, Writer& edge_writer);

private:
  bool invert_single_edge(ordering_node& from, ordering_node& to);
  ordering_node* next_train_node(const ordering_node& node);
  ordering_node* prev_train_node(const ordering_node& node);
  Writer<StringBuffer> changed_edges;
  Writer<StringBuffer> deleted_edges;
  Writer<StringBuffer> added_edges;
  Writer<StringBuffer> nodes_writer;
  Writer<StringBuffer> edges_writer;
  template <typename Writer>
  void write_edge(Writer& writer, ordering_node::id from, ordering_node::id to);
};

ordering_graph generate_testgraph(const int train_amnt, const int track_amnt,
                                  const int min_nodes, const int max_nodes);

ordering_graph generate_testgraph(const int train_amnt, const int track_amnt,
                                  const int min_nodes, const int max_nodes,
                                  const unsigned int seed);

ordering_graph from_json(const string& json);

}  // namespace soro::simulation