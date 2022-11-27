#include "doctest/doctest.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/parsers/iss/get_infra_stats.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::infra;

namespace soro::infra::test {

void check_infrastructure_stats(infrastructure_options const& opts) {
  auto const iss_files = get_iss_files(opts.infrastructure_path_);
  auto const infra_stats = get_infra_stats(iss_files);

  infrastructure const infra(opts);

  infra_stats::element_counts_t infra_element_numbers;
  std::fill(std::begin(infra_element_numbers), std::end(infra_element_numbers),
            0);

  CHECK_EQ(infra_stats.stations_, infra->stations_.size());
  CHECK_EQ(infra_stats.sections_, infra->graph_.sections_.size());
  CHECK_EQ(infra_stats.total_elements(), infra->graph_.elements_.size());

  for (auto const& e : infra->graph_.elements_) {
    ++(infra_element_numbers[type_to_id(e->type())]);
  }

  for (auto const t : all_types()) {
    CHECK_EQ(infra_element_numbers[type_to_id(t)], infra_stats.number(t));
  }
}

TEST_CASE("get_infra_stats") {
  for (auto const& opts : soro::test::ALL_INFRA_OPTS) {
    check_infrastructure_stats(opts);
  }
}

}  // namespace soro::infra::test
