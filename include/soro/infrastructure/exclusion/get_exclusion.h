#pragma once

#include "soro/infrastructure/exclusion/exclusion.h"
#include "soro/infrastructure/infrastructure_t.h"

namespace soro::infra {

exclusion get_exclusion(infrastructure_t const& infra_t,
                        std::filesystem::path const& clique_path,
                        option<exclusion_sets_tag> const exclusion_sets);

}  // namespace soro::infra
