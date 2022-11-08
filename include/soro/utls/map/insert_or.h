#pragma once

#include "soro/base/soro_types.h"

namespace soro::utls {

template <typename K, typename V, typename Fun>
constexpr void insert_or(soro::map<K, V>& map, K const& key, V&& v, Fun&& fun) {
  using std::end;

  if (auto it = map.find(key); it == end(map)) {
    map.emplace(key, v);
  } else {
    fun(it->second, std::forward<V>(v));
  }
}

}  // namespace soro::utls