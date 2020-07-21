#include "soro/unixtime.h"

#include "date/date.h"

namespace soro {

std::ostream& operator<<(std::ostream& out, unixtime const& t) {
  return out << format_unix_time(t.t_);
}

std::string format_unix_time(time_t const t, char const* format) {
  return t == std::numeric_limits<time_t>::max()
             ? "MAX"
             : date::format(format, std::chrono::system_clock::time_point{
                                        std::chrono::seconds{t}});
}

}  // namespace soro