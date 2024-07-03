#include "soro/server/modules/timetable/timetable_module.h"

#include "cereal/cereal.hpp"

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/base/time.h"

#include "soro/utls/sassert.h"

#include "soro/timetable/interval.h"
#include "soro/timetable/timetable.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

namespace soro::tt {

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar, interval const& interval) {
  ar(cereal::make_nvp("start", absolute_time_to_rep(interval.start_)),
     cereal::make_nvp("end", absolute_time_to_rep(interval.end_)));
}

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& ar, timetable const& tt) {
  ar(cereal::make_nvp("source", tt->source_),
     cereal::make_nvp("interval", tt->interval_));
}

}  // namespace soro::tt

namespace soro::server {

net::web_server::string_res_t timetable_module::serve_timetable(
    net::query_router::route_request const& req) const {

  // infrastructure_name, timetable_name
  utls::expect(req.path_params_.size() == 2);

  auto const infrastructure_name = req.path_params_.front();
  auto const timetable_name = req.path_params_.back();

  auto const timetable = get_timetable(infrastructure_name, timetable_name);

  if (!timetable.has_value()) {
    return net::not_found_response(req);
  }

  json_archive archive;
  archive.add()(cereal::make_nvp("timetable", (**timetable)));
  return json_response(req, archive);
}

}  // namespace soro::server
