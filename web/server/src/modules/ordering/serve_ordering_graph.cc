#include "soro/server/modules/ordering/ordering_module.h"

#include "utl/timer.h"

#include "net/web_server/responses.h"

#include "soro/simulation/ordering_graph.h"
#include "soro/utls/result.h"
#include "soro/utls/string.h"

#include "soro/server/cereal/json_archive.h"

namespace soro::server {

std::string serialize_ordering_graph(simulation::ordering_graph const& og) {
  using namespace rapidjson;

  utl::scoped_timer const timer("serializing ordering graph");

  StringBuffer node_buffer;
  Writer<StringBuffer> node_writer(node_buffer);
  node_writer.StartArray();

  StringBuffer edge_buffer;
  Writer<StringBuffer> edge_writer(edge_buffer);
  edge_writer.StartArray();

  for (auto const& [trip, trip_nodes] : og.trip_to_nodes_) {
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
  }

  node_writer.EndArray();
  edge_writer.EndArray();

  std::stringstream result;
  result << R"({ "attributes": {}, "nodes":)" << node_buffer.GetString()
         << R"(, "edges":)" << edge_buffer.GetString() << "}";
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

  return json_response(req, serialize_ordering_graph(ordering_graph));
}

}  // namespace soro::server