#pragma once

#include "soro/utls/unixtime.h"

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/runtime/runtime_physics.h"
#include "soro/timetable/timetable.h"

namespace soro::runtime {

struct timestamp {
  timestamp() = delete;
  timestamp(utls::unixtime const arrival, utls::unixtime const departure,
            infra::element_ptr element)
      : arrival_{arrival}, departure_{departure}, element_{element} {}

  utls::unixtime arrival_;
  utls::unixtime departure_;
  infra::element_ptr element_{nullptr};
};

struct timestamps {
  std::vector<timestamp> times_;
  // every timestamp of a HALT in times_ will be indexed in halt_indices_.
  std::vector<size_t> halt_indices_;
};

/*
 * For every element type given in record_types the returned timestamps contain
 * a timestamp with arrival and departure value.
 */
timestamps runtime_calculation(tt::train const& train,
                               infra::infrastructure const& infra,
                               infra::type_set const& record_types);

struct scenario_result {
  utls::unixtime eotd_time_{utls::INVALID_TIME};
  utls::unixtime exit_time_{utls::INVALID_TIME};
  utls::unixtime arrival_{utls::INVALID_TIME};
  utls::unixtime departure_{utls::INVALID_TIME};
  si::speed end_speed_{si::INVALID<si::speed>};
};

struct discrete_scenario {
  utls::unixtime eotd_arrival_;
  utls::unixtime end_arrival_;
  si::speed end_speed_;
};

scenario_result runtime_calculation(tt::train const& tr,
                                    infra::ir_id const ir_to_calculate,
                                    utls::unixtime const init_time,
                                    si::speed const init_speed,
                                    utls::duration const halt_duration,
                                    utls::unixtime const go_time,
                                    infra::infrastructure const&);

discrete_scenario runtime_calculation_ssr(
    tt::train const& tr, infra::infrastructure const& infra,
    infra::ir_id const ir_id, utls::unixtime const start_time,
    si::speed const init_speed, utls::unixtime const go_time,
    utls::duration const extra_stand_time);

}  // namespace soro::runtime
