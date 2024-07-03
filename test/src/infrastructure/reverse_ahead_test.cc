#include "test/infrastructure/reverse_ahead_test.h"

#include <cstddef>

#include "doctest/doctest.h"

#include "soro/base/soro_types.h"

#include "soro/infrastructure/infrastructure.h"

namespace soro::infra::test {

void check_reverse_ahead(infrastructure const& infra) {
  size_t nulls = 0;
  size_t ends = 0;

  for (auto const& element : infra->graph_.elements_) {
    ends += static_cast<std::size_t>(element->is_end_element());

    for (auto const& node : element->nodes()) {
      auto const rev = node->reverse_ahead();
      if (rev == nullptr) {
        ++nulls;
      }
    }
  }

  // TODO(julian) check if the path is correct

  CHECK_EQ(nulls, ends);
}

void do_reverse_ahead_tests(infrastructure const& infra) {
  check_reverse_ahead(infra);
}

}  // namespace soro::infra::test
