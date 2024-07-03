#pragma once

#include <vector>

#include "soro/infrastructure/graph/type.h"

namespace soro::infra {

struct type_set {
  type_set() = delete;

  template <typename Iterable>
  type_set(Iterable const& i) : set_{std::cbegin(i), std::cend(i)} {}
  type_set(std::initializer_list<type> list);

  static type_set all();
  static type_set empty();

  auto begin() const { return std::cbegin(set_); }
  auto end() const { return std::cend(set_); }

  bool contains(type const t) const;

private:
  std::vector<type> set_;
};

}  // namespace soro::infra
