#include "doctest/doctest.h"

#include <cstddef>
#include <filesystem>

#include "soro/utls/std_wrapper/fill.h"

#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/infra_stats.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/infrastructure_options.h"
#include "soro/infrastructure/parsers/iss/get_infra_stats.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

#include "test/file_paths.h"

namespace soro::infra::test {

void check_infrastructure_stats(infrastructure_options const& opts) {
  iss_files const iss_files(opts.infrastructure_path_);
  auto const infra_stats = get_infra_stats(iss_files);

  infrastructure const infra(opts);

  infra_stats::element_counts_t infra_element_numbers;
  utls::fill(infra_element_numbers, 0);

  CHECK_EQ(infra_stats.stations_, infra->stations_.size());
  CHECK_EQ(infra_stats.sections_, infra->graph_.sections_.size());

  std::size_t meta = 0;
  for (auto const& e : infra->graph_.elements_) {
    if (e->is(type::META)) {
      ++meta;
      continue;
    }

    ++(infra_element_numbers[type_to_id(e->type())]);
  }

  CHECK_EQ(infra_stats.total_elements(), infra->graph_.elements_.size() - meta);

  for (auto const t : all_types()) {
    CHECK_EQ(infra_element_numbers[type_to_id(t)], infra_stats.number(t));
  }
}

TEST_CASE("get_infra_stats") {
  for (auto const& opts : soro::test::ALL_INFRA_OPTS) {
    if (!std::filesystem::exists(opts.infrastructure_path_)) continue;
    check_infrastructure_stats(opts);
  }
}

}  // namespace soro::infra::test
