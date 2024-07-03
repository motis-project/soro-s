#include "soro/runtime/common/get_stop_time.h"

#include <algorithm>

#include "soro/base/time.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/type.h"

#include "soro/runtime/common/conversions.h"
#include "soro/runtime/common/interval.h"
#include "soro/runtime/common/signal_time.h"

#include "soro/timetable/train.h"

namespace soro::runtime {

using namespace soro::tt;

si::time get_stop_time_on_timetable_stop(interval const& interval,
                                         relative_time const arrival) {
  if (!interval.ends_on_stop()) return si::time::zero();

  auto const earliest_departure = arrival + interval.min_stop_time();
  // additional stops don't have a planned departure time
  auto const planned_departure = interval.end_departure().has_value()
                                     ? *interval.end_departure()
                                     : earliest_departure;

  auto const departure = std::max(earliest_departure, planned_departure);

  auto const result = to_si(departure - arrival);

  return result;
}

si::time get_stop_time_on_signal_stop(interval const& interval,
                                      relative_time const arrival,
                                      train::trip const& trip,
                                      signal_time const& signal_time) {
  auto const signal_stop = interval.ends_on_signal(infra::type::MAIN_SIGNAL) &&
                           interval.end_signal().id_ == signal_time.main_;
  if (!signal_stop) return si::time::zero();

  auto const signal_arrival = trip.anchor_ + arrival;
  return signal_time.time_ >= signal_arrival
             ? to_si(signal_time.time_ - signal_arrival)
             : si::time::zero();
}

si::time get_stop_time(interval const& interval, relative_time const arrival,
                       train::trip const& trip, signal_time const& st) {
  auto const tt_stop_time = get_stop_time_on_timetable_stop(interval, arrival);
  auto const s_time = get_stop_time_on_signal_stop(interval, arrival, trip, st);

  return std::max(tt_stop_time, s_time);
}

}  // namespace soro::runtime
