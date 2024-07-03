#include "soro/server/modules/search/search_module.h"

#include <string>
#include <vector>

#include "cereal/archives/json.hpp"
#include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"  // NOLINT

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "utl/to_vec.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"

namespace soro::server {

// make prologue and epilogue no-ops to be able to serialize a
// vector of search results without a root object
void prologue(cereal::JSONOutputArchive&,
              std::vector<search_module::result> const&) {}

void epilogue(cereal::JSONOutputArchive&,
              std::vector<search_module::result> const&) {}

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar,
                                    search_module::result const& e) {
  ar(cereal::make_nvp("type", search_module::result::type_to_string(e.type_)),
     cereal::make_nvp("name", e.name_), cereal::make_nvp("id", e.id_),
     cereal::make_nvp("boundingBox", e.bounding_box_),
     cereal::make_nvp("elementType", e.element_type_));
}

std::vector<search_module::result> search(std::string const& query,
                                          search_module::context const& context,
                                          infra::infrastructure const&) {
  return utl::to_vec(context.guesser_->guess_match(query),
                     [&](auto&& m) { return context.results_[m.index]; });
}

net::web_server::string_res_t search_module::serve_search(
    net::query_router::route_request const& req,
    infrastructure_module const& infra_m) const {

  auto const ctx = infra_m.get_context(req.path_params_.front());
  if (!ctx.has_value()) {
    return net::not_found_response(req);
  }

  auto const context_it = contexts_.find(req.path_params_.front());
  if (context_it == contexts_.end()) {
    return net::not_found_response(req);
  }

  if (req.path_params_.back().size() < 3) {
    return net::bad_request_response(req);
  }

  auto const search_result =
      search(req.path_params_.back(), context_it->second, (*ctx)->infra_);

  if (search_result.empty()) {
    return json_response(req, "[]");
  }

  json_archive archive;
  archive.add()(search_result);
  return json_response(req, archive);
}

}  // namespace soro::server
