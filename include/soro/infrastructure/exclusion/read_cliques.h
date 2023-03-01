#pragma once

#include "soro/infrastructure/exclusion/exclusion_set.h"
#include "soro/infrastructure/interlocking/interlocking_route.h"

namespace soro::infra {

soro::vector<exclusion_set> read_cliques(
    std::filesystem::path const& clique_fp);

}  // namespace soro::infra
