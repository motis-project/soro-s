#include "soro/base/time.h"

#include <cstdint>
#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "utl/logging.h"
#include "utl/timer.h"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/base/soro_types.h"

#include "soro/utls/parse_int.h"
#include "soro/utls/result.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/sort.h"
#include "soro/utls/string.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/train.h"

#include "soro/ordering/graph.h"

#include "soro/server/cereal/json_archive.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"

#include "soro/server/modules/timetable/timetable_module.h"

#include "soro/server/modules/ordering/ordering_module.h"

namespace soro::server {

std::string serialize_ordering_graph(ordering::graph const& og,
                                     tt::timetable const& tt) {
  using namespace rapidjson;

  utl::scoped_timer const timer("serializing ordering graph");

  auto trips_copy = og.trips_;

  utls::sort(trips_copy, [&](auto&& p1, auto&& p2) {
    auto const& t1 = tt->trains_[p1.train_id_];
    auto const& t2 = tt->trains_[p2.train_id_];
    return soro::relative_to_absolute(p1.anchor_, t1.start_time_) <
           soro::relative_to_absolute(p2.anchor_, t2.start_time_);
  });

  uint32_t total_trains = 0;
  uint32_t max_train_length = 0;

  StringBuffer node_buffer;  // NOLINT
  Writer<StringBuffer> node_writer(node_buffer);  // NOLINT
  node_writer.StartArray();

  StringBuffer edge_buffer;  // NOLINT
  Writer<StringBuffer> edge_writer(edge_buffer);  // NOLINT
  edge_writer.StartArray();

  StringBuffer graph_attributes_buffer;  // NOLINT
  Writer<StringBuffer> graph_attributes_writer(  // NOLINT
      graph_attributes_buffer);
  graph_attributes_writer.StartObject();

  tt::train::id relative_train_id = 0;
  for (auto const& trip : trips_copy) {
    ++total_trains;

    max_train_length = std::max(max_train_length, (trip.to_ - trip.from_).v_);

    for (auto n_id = trip.from_; n_id < trip.to_; ++n_id) {
      auto const& node = og.nodes_[n_id];

      node_writer.StartObject();
      node_writer.String("key");
      node_writer.Uint(node.get_id(og).v_);

      node_writer.String("attributes");
      node_writer.StartObject();

      node_writer.String("trip");
      node_writer.Uint(node.trip_id_.v_);

      node_writer.String("train");
      node_writer.Uint(og.trips_[node.trip_id_].train_id_);

      node_writer.String("offset");
      node_writer.Uint(node.get_id(og).v_ - trip.from_.v_);

      node_writer.String("relativeTrainId");
      node_writer.Uint(relative_train_id);

      node_writer.String("route");
      node_writer.Uint(as_val(node.ir_id_));

      node_writer.EndObject();

      node_writer.EndObject();
      for (auto const to : node.out(og)) {
        edge_writer.StartObject();
        edge_writer.String("source");
        edge_writer.Uint(node.get_id(og).v_);
        edge_writer.String("target");
        edge_writer.Uint(to.v_);
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

  uLOG(utl::info) << "serialized ordering graph size: "
                  << (result.str().size() / (1024UL * 1024UL)) << " mB";

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

utls::result<ordering::graph::filter> params_to_filter(
    std::vector<std::string> const& params) {
  ordering::graph::filter filter;

  auto const from = str_to_absolute_time(params[2]);
  if (!from) return utls::propagate(from);
  filter.interval_.start_ = *from;

  auto const to = str_to_absolute_time(params[3]);
  if (!to) return utls::propagate(to);
  filter.interval_.end_ = *to;

  if (!filter.interval_.valid()) {
    return utls::unexpected(std::errc::invalid_argument);
  }

  auto train_ids = comma_values_to_train_ids(params[4]);
  if (!train_ids) return utls::propagate(train_ids);
  filter.include_trains_ = std::move(*train_ids);

  return filter;
}

// no static
// NOLINTNEXTLINE
net::web_server::string_res_t ordering_module::serve_ordering_graph(
    net::query_router::route_request const& req,
    infrastructure_module const& infra_m,
    timetable_module const& timetable_m) const {
  utls::expect(req.path_params_.size() == 5);

  auto const& infra_name = req.path_params_.front();
  auto const ctx = infra_m.get_context(infra_name);
  if (!ctx.has_value()) return net::not_found_response(req);

  auto const& tt_name = req.path_params_[1];
  auto const tt = timetable_m.get_timetable(infra_name, tt_name);
  if (!tt.has_value()) return net::not_found_response(req);

  auto const filter = params_to_filter(req.path_params_);
  if (!filter) return net::bad_request_response(req);

  ordering::graph const ordering_graph((*ctx)->infra_, **tt, *filter);

  return json_response(req, serialize_ordering_graph(ordering_graph, **tt));
}

}  // namespace soro::server