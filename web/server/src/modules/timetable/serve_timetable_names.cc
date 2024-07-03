#include "soro/server/modules/timetable/timetable_module.h"

#include <range/v3/view/transform.hpp>

#include "cereal/cereal.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/web_server.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

net::web_server::string_res_t timetable_module::serve_timetable_names(
    net::query_router::route_request const& req) const {
  auto const& timetables = all(req.path_params_.front());

  json_archive archive;
  archive.add()(cereal::make_nvp(
      "timetables", timetables | ranges::views::transform(
                                     [](auto&& t) { return t->source_; })));

  return json_response(req, archive);
}

}  // namespace soro::server