#pragma once

#include "soro/timetable/timetable_options.h"
#include "soro/timetable/train.h"

namespace soro::tt {

struct base_timetable {
  utls::unixtime start_;
  utls::unixtime end_;

  soro::vector<train> trains_;
  soro::map<train::number, train::ptr> number_to_train_;
};

base_timetable make_base_timetable(soro::vector<soro::unique_ptr<train>> const&,
                                   timetable_options const& opts);

}  // namespace soro::tt
