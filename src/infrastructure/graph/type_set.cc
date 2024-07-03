#include "soro/infrastructure/graph/type_set.h"

#include <initializer_list>

#include "soro/utls/std_wrapper/contains.h"

#include "soro/infrastructure/graph/type.h"

namespace soro::infra {

type_set::type_set(std::initializer_list<type> list) : set_{list} {}

type_set type_set::all() { return type_set{all_types()}; }

type_set type_set::empty() { return type_set{{}}; }

bool type_set::contains(type const t) const { return utls::contains(set_, t); }

}  // namespace soro::infra