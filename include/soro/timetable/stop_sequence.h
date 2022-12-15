#pragma once

#include "soro/timetable/sequence_point.h"

namespace soro::tt {

struct stop_sequence {
  std::vector<sequence_point> points_;
  bool break_in_;
  bool break_out_;
};

}  // namespace soro::tt