#pragma once

#include <algorithm>
#include <vector>

namespace soro::utls {

template <typename Vector, typename Pred>
inline void remove_until(Vector& v, Pred&& pred) {
  auto const first = std::find_if(std::begin(v), std::end(v), pred);
  v.erase(std::begin(v), first);
}

}  // namespace soro::utls
