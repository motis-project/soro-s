#pragma once

#include "soro/infrastructure/brake_tables.h"
#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

namespace soro::infra {

brake_tables get_brake_tables(iss_files const& iss_files,
                              dictionaries const& dicts);

}  // namespace soro::infra