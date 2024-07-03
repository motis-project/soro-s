#pragma once

#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/layout.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::infra {

layout parse_layout(iss_files const& iss_files, infrastructure_t const& infra,
                    construction_materials const& mats);

}  // namespace soro::infra
