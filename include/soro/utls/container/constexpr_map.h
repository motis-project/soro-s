#pragma once

#include <algorithm>
#include <array>

namespace soro::utls {

// taken from jason turners constexpr map
template <typename Key, typename Value, std::size_t Size>
struct constexpr_map {
  [[nodiscard]] constexpr Value at(const Key& key) const {
    const auto itr =
        std::find_if(begin(data_), end(data_),
                     [&key](const auto& v) { return v.first == key; });
    if (itr != end(data_)) {
      return itr->second;
    } else {
      throw std::range_error("Not Found");
    }
  }

  std::array<std::pair<Key, Value>, Size> data_;
};

}  // namespace soro::utls