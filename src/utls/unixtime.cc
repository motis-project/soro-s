#include "soro/utls/unixtime.h"

namespace soro::utls {

const duration duration::INVALID = duration{duration::INVALID_VALUE};
const duration duration::ZERO = duration{0};

std::ostream& operator<<(std::ostream& out, duration const& d) {
  return out << d.d_ << "s";
}

std::ostream& operator<<(std::ostream& out, unixtime const& t) {
  return out << format_unix_time(t);
}

std::string format_unix_time(unixtime const t, char const* format) {
  return t.t_ == std::numeric_limits<time_t>::max()
             ? "MAX"
             : date::format(format, std::chrono::system_clock::time_point{
                                        std::chrono::seconds{t.t_}});
}

bool unixtime::in_interval(unixtime const start, unixtime const end) const {
  return *this >= start && *this <= end;
}

}  // namespace soro::utls