#pragma once

#include "soro/infrastructure/exclusion/exclusion.h"
#include "soro/infrastructure/infrastructure_t.h"

namespace soro::infra {

exclusion get_exclusion(infrastructure_t const& infra_t,
                        std::filesystem::path const& clique_path,
                        option<exclusion_elements> const exclusion_elements,
                        option<exclusion_graph> const exclusion_graph);

}  // namespace soro::infra
