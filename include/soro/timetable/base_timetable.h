#pragma once

#include "soro/timetable/timetable_options.h"
#include "soro/timetable/train.h"

namespace soro::tt {

struct base_timetable {
  soro::vector<train> trains_;

  soro::map<train::number, train::ptr> number_to_train_;

  interval interval_{};
  soro::string source_;
};

}  // namespace soro::tt
