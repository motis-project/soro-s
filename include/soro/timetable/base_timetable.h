#pragma once

#include "soro/timetable/connection.h"
#include "soro/timetable/timetable_options.h"
#include "soro/timetable/train.h"

namespace soro::tt {

struct base_timetable {
  soro::vector<train> trains_;
  // same size as trains_
  soro::vector<connection> connections_;

  soro::map<train::number, train::ptr> number_to_train_;

  interval interval_{};
  soro::string source_;
};

}  // namespace soro::tt
