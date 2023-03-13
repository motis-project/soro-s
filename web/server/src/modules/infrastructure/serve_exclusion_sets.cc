#include "soro/server/modules/infrastructure/infrastructure.h"

#include "cereal/types/vector.hpp"

#include "net/web_server/responses.h"

#include "soro/server/cereal/json_archive.h"

namespace soro::infra {

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive, exclusion_set const& es) {
  archive(cereal::make_nvp("id", es.id_), cereal::make_nvp("size", es.count()));
}

}  // namespace soro::infra

namespace soro::server {

net::web_server::string_res_t infra_state::serve_exclusion_sets(
    net::query_router::route_request const& req) const {

  auto const infra = get_infra(req.path_params_.front());
  if (!infra.has_value()) {
    return net::not_found_response(req);
  }

  json_archive archive;
  archive.add()(cereal::make_nvp("exclusion_sets",
                                 (**infra)->exclusion_.exclusion_sets_));
  return json_response(req, archive);
}

}  // namespace soro::server
