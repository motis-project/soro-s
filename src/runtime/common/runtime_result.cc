#include "soro/runtime/common/runtime_result.h"

#include <ostream>

namespace soro::runtime {

runtime_result& runtime_result::operator+=(runtime_result const& other) {
  time_ += other.time_;
  dist_ += other.dist_;
  speed_ += other.speed_;
  return *this;
}

inline std::ostream& operator<<(std::ostream& out, runtime_result const& rr) {
  out << "runtime result:\n"
      << "time: " << rr.time_ << " [s]\n"
      << "distance: " << rr.dist_ << " [m]\n"
      << "speed: " << rr.speed_ << '\n';
  return out;
}

inline std::ostream& operator<<(std::ostream& out, runtime_results const& rr) {
  for (auto const& entry : rr) {
    out << entry;
  }
  return out;
}

}  // namespace soro::runtime
