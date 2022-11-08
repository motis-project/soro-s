#include "doctest/doctest.h"

#include <set>

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

void check_element_uniqueness(train const& t) {
  std::set<element_id> elements;

  for (auto const& rn : t.iterate(skip_omitted::ON)) {
    auto const& [ignore, success] = elements.emplace(rn.node_->element_->id());
    CHECK_MESSAGE(success,
                  "A train iterator should visit an element only once.");
  }
}

void check_node_uniqueness(train const& t) {
  std::set<node::id> nodes;
  for (auto const& rn : t.iterate(skip_omitted::ON)) {
    auto const& [ignore, success] = nodes.emplace(rn.node_->id_);
    CHECK_MESSAGE(success, "A train iterator should visit a node only once.");
  }
}

// TODO(julian) Check for correctly omitted nodes

void check_train_iterator(train const& t) {
  check_element_uniqueness(t);
  check_node_uniqueness(t);
}

TEST_SUITE("train iterator") {
  TEST_CASE("train iterator - follow") {
    infrastructure const infra(SMALL_OPTS);
    timetable const tt(FOLLOW_OPTS, infra);

    for (auto const& t : tt) {
      check_train_iterator(*t);
    }
  }
}