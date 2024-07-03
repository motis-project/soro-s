#pragma once

#include "soro/si/units.h"

namespace soro::runtime {

struct runtime_result {
  runtime_result() = default;
  runtime_result(si::time const seconds, si::length const meter,
                 si::speed const m_s)
      : time_{seconds}, dist_{meter}, speed_{m_s} {}

  runtime_result& operator+=(runtime_result const& other);

  si::time time_{si::time::zero()};
  si::length dist_{si::length::zero()};
  si::speed speed_{si::speed::zero()};
};

using runtime_results = std::vector<runtime_result>;

std::ostream& operator<<(std::ostream& out, runtime_result const& rr);
std::ostream& operator<<(std::ostream& out, runtime_results const& rr);


}  // namespace soro::runtime