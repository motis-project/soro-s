#pragma once

#include <algorithm>
#include <vector>

namespace soro::utls {

template <typename Vector, typename Pred>
inline void remove_after(Vector& v, Pred&& pred) {
  auto const rlast = std::find_if(std::rbegin(v), std::rend(v), pred);
  auto const last = std::begin(v) + std::distance(rlast, std::rend(v));
  v.erase(last, std::end(v));
}

}  // namespace soro::utls
