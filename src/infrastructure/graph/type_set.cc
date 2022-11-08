#include "soro/infrastructure/graph/type_set.h"

#include "utl/to_vec.h"

namespace soro::infra {

type_set::type_set(std::initializer_list<type> list) : set_{list} {}

type_set::type_set(decltype(all_types()) const all)
    : set_(utl::to_vec(all, [](auto&& t) { return t; })) {}

bool type_set::contains(type const t) const {
  return std::find(std::cbegin(set_), std::cend(set_), t) != std::cend(set_);
}

}  // namespace soro::infra