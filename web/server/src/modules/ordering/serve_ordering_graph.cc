#include "soro/base/time.h"

#include "utl/timer.h"

#include "net/web_server/responses.h"

#include "soro/utls/result.h"
#include "soro/utls/std_wrapper/sort.h"
#include "soro/utls/string.h"

#include "soro/simulation/ordering/ordering_graph.h"

#include "soro/server/cereal/json_archive.h"
#include "soro/server/modules/ordering/ordering_module.h"

namespace soro::server {

std::string serialize_ordering_graph(simulation::ordering_graph const& og,
                                     tt::timetable const& tt) {
  using namespace rapidjson;

  utl::scoped_timer const timer("serializing ordering graph");

  std::vector<
      std::pair<tt::train::trip, std::pair<simulation::ordering_node::id,
                                           simulation::ordering_node::id>>>
      cpy(std::begin(og.trip_to_nodes_), std::end(og.trip_to_nodes_));

  utls::sort(cpy, [&](auto&& p1, auto&& p2) {
    auto const& t1 = tt->trains_[p1.first.train_id_];
    auto const& t2 = tt->trains_[p2.first.train_id_];
    return soro::relative_to_absolute(p1.first.anchor_, t1.first_departure()) <
           soro::relative_to_absolute(p2.first.anchor_, t2.first_departure());
  });

  uint32_t total_trains = 0;
  uint32_t max_train_length = 0;

  StringBuffer node_buffer;
  Writer<StringBuffer> node_writer(node_buffer);
  node_writer.StartArray();

  StringBuffer edge_buffer;
  Writer<StringBuffer> edge_writer(edge_buffer);
  edge_writer.StartArray();

  StringBuffer graph_attributes_buffer;
  Writer<StringBuffer> graph_attributes_writer(graph_attributes_buffer);
  graph_attributes_writer.StartObject();

  tt::train::id relative_train_id = 0;
  for (auto const& [trip, trip_nodes] : cpy) {
    ++total_trains;

    max_train_length =
        std::max(max_train_length, trip_nodes.second - trip_nodes.first);

    for (auto n_id = trip_nodes.first; n_id < trip_nodes.second; ++n_id) {
      auto const& node = og.nodes_[n_id];

      node_writer.StartObject();
      node_writer.String("key");
      node_writer.Uint(node.id_);

      node_writer.String("attributes");
      node_writer.StartObject();

      node_writer.String("train");
      node_writer.Uint(node.train_id_);

      node_writer.String("offset");
      node_writer.Uint(node.id_ - trip_nodes.first);

      node_writer.String("relativeTrainId");
      node_writer.Uint(relative_train_id);

      node_writer.String("route");
      node_writer.Uint(node.ir_id_);

      node_writer.EndObject();

      node_writer.EndObject();
      for (auto const to : node.out_) {
        edge_writer.StartObject();
        edge_writer.String("source");
        edge_writer.Uint(node.id_);
        edge_writer.String("target");
        edge_writer.Uint(to);
        edge_writer.EndObject();
      }
    }

    ++relative_train_id;
  }

  graph_attributes_writer.String("totalTrains");
  graph_attributes_writer.Uint(total_trains);

  graph_attributes_writer.String("maxTrainLength");
  graph_attributes_writer.Uint(max_train_length);

  node_writer.EndArray();
  edge_writer.EndArray();
  graph_attributes_writer.EndObject();

  std::stringstream result;
  result << R"({ "attributes": )" << graph_attributes_buffer.GetString()
         << R"(, "nodes":)" << node_buffer.GetString() << R"(, "edges":)"
         << edge_buffer.GetString() << "}";

  uLOG(utl::info) << "serialized ordering graph size: " << result.str().size();

  return result.str();
}

utls::result<std::vector<tt::train::id>> comma_values_to_train_ids(
    std::string_view const csv) {
  auto const split = utls::split(csv, ",");

  std::vector<tt::train::id> result;
  result.reserve(split.size());

  for (auto const sv : utls::split(csv, ",")) {
    auto const parsed_id = utls::try_parse_int<tt::train::id>(sv);

    if (!parsed_id) return utls::propagate(parsed_id);

    result.push_back(*parsed_id);
  }

  return result;
}

// no static
// NOLINTNEXTLINE
net::web_server::string_res_t ordering_module::serve_ordering_graph(
    net::query_router::route_request const& req,
    infrastructure_module const& infra_m,
    timetable_module const& timetable_m) const {
  utls::expect(req.path_params_.size() == 5);

  std::string_view const infra_name = req.path_params_.front();

  auto const infra = infra_m.get_infra(infra_name);
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  std::string_view const timetable_name = req.path_params_[1];
  auto const timetable = timetable_m.get_timetable(infra_name, timetable_name);
  if (!timetable.has_value()) {
    return net::not_found_response(req);
  }

  simulation::ordering_graph::filter filter;

  auto const from = str_to_absolute_time(req.path_params_[2]);
  if (!from) return net::bad_request_response(req);
  filter.interval_.start_ = *from;

  auto const to = str_to_absolute_time(req.path_params_[3]);
  if (!to) return net::bad_request_response(req);
  filter.interval_.end_ = *to;

  if (!filter.interval_.valid()) return net::bad_request_response(req);

  auto train_ids = comma_values_to_train_ids(req.path_params_[4]);
  if (!train_ids) return net::bad_request_response(req);
  filter.trains_ = std::move(*train_ids);

  simulation::ordering_graph const ordering_graph(**infra, **timetable, filter);

  return json_response(req,
                       serialize_ordering_graph(ordering_graph, **timetable));
}

}  // namespace soro::server