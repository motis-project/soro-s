#pragma once

#include <ctime>
#include <iosfwd>
#include <vector>

#include "cista/containers/hash_map.h"
#include "cista/containers/unique_ptr.h"

namespace rapid {

struct node;

struct train {
  friend std::ostream& operator<<(std::ostream&, train const&);
  std::string name_;
  float speed_;
  std::vector<std::pair<time_t, node*>> timetable_;
};

using timetable =
    cista::raw::hash_map<std::string, cista::raw::unique_ptr<train>>;

}  // namespace rapid
