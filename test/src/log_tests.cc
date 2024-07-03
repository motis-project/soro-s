#include "doctest/doctest.h"

#include <set>

#include "utl/logging.h"

#include "soro/base/soro_types.h"

#include "soro/utls/execute_if.h"
#include "soro/utls/narrow.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro::utls;
using namespace soro::tt;
using namespace soro::infra;
using namespace utl;

void log_main_signal_in_station_routes_stats(infrastructure const& infra) {
  soro::map<soro::size_t, soro::size_t> ms_count_to_sr_count;

  for (auto const& sr : infra->station_routes_) {
    auto const ms_it =
        ms_count_to_sr_count.find(sr->path_->main_signals_.size());
    if (ms_it != std::end(ms_count_to_sr_count)) {
      ++(ms_it->second);
    } else {
      ms_count_to_sr_count[narrow<soro::size_t>(
          sr->path_->main_signals_.size())] = 1;
    }
  }

  uLOG(info) << "Main signal in station route stats:";

  soro::size_t check_count = 0;
  for (auto const& [ms_count, sr_count] : ms_count_to_sr_count) {
    uLOG(info) << "Station routes with " << ms_count
               << " main signals: " << sr_count;
    check_count += sr_count;
  }

  CHECK_EQ(check_count, infra->station_routes_.size());
}

void log_possible_speed_limit_values(infrastructure const& infra) {
  std::set<soro::si::speed> possible_values;

  for (auto const& data : infra->graph_.element_data_) {
    execute_if<speed_limit>(data, [&](auto&& spl) {
      if (spl.limit_.is_valid()) {
        possible_values.insert(spl.limit_);
      }
    });
  }

  uLOG(info) << "Possible speed limit values:";
  for (auto const& value : possible_values) {
    uLOG(info) << value;
  }
  uLOG(info) << "In total: " << possible_values.size();
}

void log_timetable_stats(timetable const& tt) {
  soro::size_t avg_length_in_irs = 0;
  soro::size_t avg_service_days = 0;

  for (auto const& train : tt->trains_) {
    avg_length_in_irs += train.path_.size();
    avg_service_days += train.service_days_.count();
  }

  uLOG(info) << "Average train length in interlocking routes: "
             << avg_length_in_irs / tt->trains_.size();
  uLOG(info) << "Average train service days: "
             << avg_service_days / tt->trains_.size();
}

void logs(infrastructure const& infra, timetable const& tt) {
  log_main_signal_in_station_routes_stats(infra);
  log_possible_speed_limit_values(infra);
  log_timetable_stats(tt);
}

TEST_CASE("log infrastructure stats") {  // NOLINT
  for (auto const& scenario : soro::test::get_timetable_scenarios()) {
    logs(*scenario->infra_, scenario->timetable_);
  }
}
