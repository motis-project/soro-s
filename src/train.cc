#include "rapid/train.h"

#include <ostream>

#include "utl/enumerate.h"

#include "date/date.h"

#include "rapid/network.h"

namespace rapid {

std::ostream& operator<<(std::ostream& out, train const& t) {
  out << "{ name=" << t.name_ << ", speed=" << t.speed_ << ", timetable=[";
  for (auto const [i, entry] : utl::enumerate(t.timetable_)) {
    if (i != 0) {
      out << ", ";
    }
    out << entry.second->name_ << " "
        << date::format("%F %T",
                        date::sys_seconds{std::chrono::seconds{entry.first}});
  }
  return out << "] }";
}

}  // namespace rapid