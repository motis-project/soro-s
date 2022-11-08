#pragma once

#include <vector>

#include "soro/infrastructure/graph/type.h"

namespace soro::infra {

struct type_set {
  type_set() = delete;
  type_set(std::initializer_list<type> list);
  explicit type_set(decltype(all_types()) const all);

  auto begin() const { return std::cbegin(set_); }
  auto end() const { return std::cend(set_); }

  bool contains(type const t) const;

private:
  std::vector<type> set_;
};

}  // namespace soro::infra
