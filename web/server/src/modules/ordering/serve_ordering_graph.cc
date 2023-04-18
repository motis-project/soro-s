#include "soro/server/modules/ordering/ordering_module.h"

#include "net/web_server/responses.h"

#include "soro/simulation/ordering_graph.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"

namespace soro::simulation {

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive, ordering_node const& on) {
  archive(cereal::make_nvp("key", on.id_));
}

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive,
                                    ordering_graph const& og) {

  archive(cereal::make_nvp("nodes", og.nodes_));

  for (auto const& from : og.nodes_) {
    for (auto const& to : from.out_) {
    }
  }
}

}  // namespace soro::simulation

namespace soro::server {

net::web_server::string_res_t ordering_module::serve_ordering_graph(
    net::query_router::route_request const& req,
    infrastructure_module const& infra_m,
    timetable_module const& timetable_m) const {

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

  simulation::ordering_graph const ordering_graph(**infra, **timetable);

  json_archive archive;
  archive.add()(cereal::make_nvp("ordering_graph", ordering_graph));
  return json_response(req, archive);
}

}  // namespace soro::server