#pragma once

#include "doctest/doctest.h"

#include "utl/pairwise.h"

#include "soro/base/soro_types.h"

namespace soro::test::utls {

template <typename Container>
void check_continuous_ascending_ids(Container const& c) {
  for (auto [e1, e2] : utl::pairwise(c)) {
    if constexpr (soro::is_pointer<std::remove_reference_t<decltype(e1)>>()) {
      CHECK(e1->id_ + 1 == e2->id_);
    } else {
      CHECK(e1.id_ + 1 == e2.id_);
    }
  }

  if constexpr (soro::is_pointer<
                    std::remove_reference_t<decltype(*std::begin(c))>>()) {
    CHECK_EQ((*std::begin(c))->id_, 0);
    CHECK_EQ((*(std::end(c) - 1))->id_, c.size() - 1);
  } else {
    CHECK_EQ(*std::begin(c).id_, 0);
    CHECK_EQ(*(std::end(c) - 1).id_, c.size() - 1);
  }
}

}  // namespace soro::test::utls
