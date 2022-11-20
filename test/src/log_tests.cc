#include "doctest/doctest.h"

#include <set>

#include "utl/logging.h"

#include "soro/utls/execute_if.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::utls;
using namespace soro::tt;
using namespace soro::infra;
using namespace utl;

TEST_SUITE_BEGIN("log_test_suite");  // NOLINT

void log_main_signal_in_station_routes_stats(infrastructure const& infra) {
  soro::map<soro::size_type, soro::size_type> ms_count_to_sr_count;

  for (auto const& sr : infra->station_routes_) {
    auto const ms_it = ms_count_to_sr_count.find(sr->main_signals_.size());
    if (ms_it != std::end(ms_count_to_sr_count)) {
      ++(ms_it->second);
    } else {
      ms_count_to_sr_count[sr->main_signals_.size()] = 1;
    }
  }

  uLOG(info) << "Main signal in station route stats:";

  soro::size_type check_count = 0;
  for (auto const& [ms_count, sr_count] : ms_count_to_sr_count) {
    uLOG(info) << "Station routes with " << ms_count
               << " main signals: " << sr_count;
    check_count += sr_count;
  }

  CHECK_EQ(check_count, infra->station_routes_.size());
}

void log_possible_speed_limit_values(infrastructure const& infra) {
  std::set<si::speed> possible_values;

  for (auto const& data : infra->graph_.element_data_) {
    execute_if<speed_limit>(data, [&](auto&& spl) {
      if (si::valid(spl.limit_)) {
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

void log_signal_station_route_halt_counts(infrastructure const&) {
  std::map<soro::size_type, soro::size_type> passenger_halt_counts;
  std::map<soro::size_type, soro::size_type> freight_halt_counts;

  utls::sassert(false, "Not implemented");
  //  for (auto const& ssr : infra->interlocking_.interlocking_routes_) {
  //    if (passenger_halt_counts.contains(ssr->passenger_halts_.size())) {
  //      ++(passenger_halt_counts[ssr->passenger_halts_.size()]);
  //    } else {
  //      passenger_halt_counts[ssr->passenger_halts_.size()] = 1;
  //    }
  //
  //    if (freight_halt_counts.contains(ssr->freight_halts_.size())) {
  //      ++(freight_halt_counts[ssr->freight_halts_.size()]);
  //    } else {
  //      freight_halt_counts[ssr->freight_halts_.size()] = 1;
  //    }
  //  }

  uLOG(info) << "Signal station route passenger halt count stats:";
  for (auto const& [passenger_halts, stat] : passenger_halt_counts) {
    uLOG(info) << "Passenger halt count: " << passenger_halts << " occured "
               << stat << " times.";
  }

  uLOG(info) << "Signal station route freight halt count stats:";
  for (auto const& [freight_halts, stat] : freight_halt_counts) {
    uLOG(info) << "Freight halt count: " << freight_halts << " occured " << stat
               << " times.";
  }
}

void print_first_departure_buckets(timetable const& tt) {
  std::map<unixtime, size_t> buckets;

  for (auto const& train_run : tt->train_store_) {
    auto const one_hour = unixtime{3600};
    auto const remainder = train_run->first_departure() % one_hour;
    auto const bucket_time = train_run->first_departure() - remainder;

    if (auto it = buckets.find(bucket_time); it != std::end(buckets)) {
      ++(it->second);
    } else {
      buckets[bucket_time] = 1;
    }
  }

  for (auto const& [bucket_time, size] : buckets) {
    uLOG(utl::info) << "Bucket Time: " << bucket_time << " Size: " << size;
  }
}

void logs(infrastructure const& infra) {
  log_main_signal_in_station_routes_stats(infra);
  log_possible_speed_limit_values(infra);
  log_signal_station_route_halt_counts(infra);
}

TEST_CASE("log infrastructure stats") {  // NOLINT
  infrastructure const infra(SMALL_OPTS);
  logs(infra);
}

TEST_SUITE_END();  // NOLINT
