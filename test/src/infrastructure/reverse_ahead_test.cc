#include "doctest/doctest.h"

#include "soro/infrastructure/infrastructure.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::infra;

bool is_final_frontier(element_ptr border) {
  auto const& b = border->as<simple_element>();
  return (b.rising_ ? b.top() : b.bot())->next_node_ == nullptr;
}

void check_reverse_ahead(base_infrastructure const& iss) {
  size_t nulls = 0;
  size_t ends = 0;

  for (auto const& element : iss.graph_.elements_) {
    ends += static_cast<size_t>(element->is_end_element());
    ends += static_cast<size_t>(element->is(type::BORDER) &&
                                is_final_frontier(element));

    for (auto const& node : element->nodes()) {
      auto const rev = node->reverse_ahead();
      if (rev == nullptr) {
        ++nulls;
      }
    }
  }

  // TODO(julian) check if the path is correct

  CHECK(nulls == ends);
}

TEST_CASE("reverse ahead") {  // NOLINT
  infrastructure const infra(SMALL_OPTS);
  check_reverse_ahead(*infra);
}
