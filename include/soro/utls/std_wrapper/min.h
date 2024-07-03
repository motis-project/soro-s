#pragma once

#include "soro/utls/std_wrapper/min_element.h"

namespace soro::utls {

template <typename Iterable>
[[nodiscard]] constexpr auto min(Iterable&& i) {
  return detail::min_element(std::forward<Iterable>(i));
}

}  // namespace soro::utls
