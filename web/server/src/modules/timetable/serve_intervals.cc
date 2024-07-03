#include "soro/server/modules/timetable/timetable_module.h"

#include "cereal/cereal.hpp"
#include "cereal/types/vector.hpp"  // NOLINT

#include "net/web_server/query_router.h"
#include "net/web_server/responses.h"
#include "net/web_server/web_server.h"

#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/type_set.h"

#include "soro/timetable/train.h"

#include "soro/runtime/common/get_intervals.h"
#include "soro/runtime/common/interval.h"

#include "soro/server/cereal/cereal_extern.h"  // NOLINT
#include "soro/server/cereal/json_archive.h"

#include "soro/server/modules/infrastructure/infrastructure_module.h"

namespace soro::runtime {

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive, interval_point const& p) {
  archive(cereal::make_nvp("dist", p.distance_));
  archive(cereal::make_nvp("limit", p.limit_));
  archive(cereal::make_nvp("bwp_limit", p.bwp_limit_));
  archive(cereal::make_nvp("signal_limit", p.signal_limit_));
  archive(cereal::make_nvp("slope", p.slope_));
  archive(cereal::make_nvp("brake_path_length", p.brake_path_length_));
}

template <typename Archive>
void CEREAL_SERIALIZE_FUNCTION_NAME(Archive& archive,
                                    intervals const& intervals) {
  archive(cereal::make_nvp("points", intervals.p_));
}

}  // namespace soro::runtime

namespace soro::server {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::runtime;

net::web_server::string_res_t timetable_module::serve_intervals(
    net::query_router::route_request const& req,
    infrastructure_module const& infra_m) const {

  // infrastructure_name, timetable_name, train_id
  utls::expect(req.path_params_.size() == 3);

  auto const infra_name = req.path_params_[0];
  auto const infra_ctx = infra_m.get_context(infra_name);
  if (!infra_ctx.has_value()) return net::not_found_response(req);
  auto const& infra = (*infra_ctx)->infra_;

  auto const tt_name = req.path_params_[1];
  auto const tt = get_timetable(infra_name, tt_name);
  if (!tt.has_value()) return net::not_found_response(req);

  auto const train_id = utls::parse_int<train::id>(req.path_params_[2]);
  if (train_id >= (**tt)->trains_.size()) return net::not_found_response(req);
  auto const& train = (**tt)->trains_[train_id];

  auto const record_types = type_set::empty();
  auto const intervals = get_intervals(train, record_types, infra);

  json_archive archive;
  archive.add()(cereal::make_nvp("intervals", intervals));
  return json_response(req, archive);
}

}  // namespace soro::server