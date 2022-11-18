#pragma once

#include "utl/zip.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

namespace soro::tt {

struct raw_train {
  struct run {
    auto entries() const {
      return utl::zip(routes_, arrivals_, departures_, min_stop_times_);
    }

    soro::vector<infra::station_route::ptr> routes_;
    soro::vector<utls::unixtime> arrivals_;
    soro::vector<utls::unixtime> departures_;
    soro::vector<utls::duration> min_stop_times_;
    soro::vector<stop> stops_;
    bool break_in_;
    bool break_out_;
  };

  struct traction_unit {
    soro::string series_;
    soro::string owner_;
    rs::variant_id variant_;
  };

  struct characteristic {
    std::vector<traction_unit> traction_units_;
    si::length length_{si::ZERO<si::length>};
    si::speed max_speed_{si::ZERO<si::speed>};
    si::weight carriage_weight_{si::ZERO<si::weight>};
    rs::FreightTrain freight_{rs::FreightTrain::NO};
  };

  std::size_t id_{std::numeric_limits<std::size_t>::max()};
  soro::string name_;

  rs::FreightTrain freight_{rs::FreightTrain::NO};
  rs::CTC ctc_{rs::CTC::NO};

  run run_;
  characteristic charac_;
};

soro::vector<soro::unique_ptr<train>> raw_trains_to_trains(
    std::vector<raw_train> const& raw_trains,
    infra::interlocking_subsystem const& ssr_man,
    infra::station_route_graph const& srg,
    rs::rolling_stock const& rolling_stock);

}  // namespace soro::tt
