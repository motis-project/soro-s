#include "doctest/doctest.h"

#include "soro/ordering/graph.h"

using namespace soro::ordering;

TEST_CASE("ordering graph vecvec") {
  soro::ordering::graph og;
  get_my_span(og.outgoing_edges_.data_.data(), 0);
}
