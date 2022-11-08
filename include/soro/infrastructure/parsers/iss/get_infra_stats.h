#pragma once

#include "soro/infrastructure/infra_stats.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::infra {

infra_stats get_infra_stats(iss_files const& files);

}  // namespace soro::infra
