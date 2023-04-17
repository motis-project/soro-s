#include "soro/server/modules/timetable/timetable_module.h"

#include "soro/server/cereal/cereal_extern.h"
#include "soro/server/cereal/json_archive.h"

namespace soro::server {

net::web_server::string_res_t timetable_module::serve_timetable_names(
    net::query_router::route_request const& req) const {
  auto const& context = this->contexts_.at(req.path_params_.front());

  json_archive archive;
  archive.add()(cereal::make_nvp("timetables",
                                 context.timetables_ | ranges::views::keys));

  return json_response(req, archive);
}

}  // namespace soro::server