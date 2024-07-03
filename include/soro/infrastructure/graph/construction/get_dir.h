#pragma once

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/section.h"

namespace soro::infra {

detail::element_array_idx get_dir(std::string const& node_name,
                                  section::position const pos);

simple_switch::direction get_switch_dir(std::string const& node_name);

cross::direction get_cross_dir(std::string const& node_name);

}  // namespace soro::infra