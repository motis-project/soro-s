#include "doctest/doctest.h"

#include <set>

#include "soro/infrastructure/infrastructure.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;

void check_element_uniqueness(train const& t, infrastructure const& infra) {
  std::set<element_id> elements;

  for (auto const& rn : t.iterate(infra)) {
    if (rn.omitted_) {
      continue;
    }

    auto const& [ignore, success] = elements.emplace(rn.node_->element_->id());
    CHECK_MESSAGE(success,
                  "A train iterator should visit an element only once.");
  }
}

void check_node_uniqueness(train const& t, infrastructure const& infra) {
  std::set<node::id> nodes;
  for (auto const& rn : t.iterate(infra)) {
    if (rn.omitted_) {
      continue;
    }
   
    auto const& [ignore, success] = nodes.emplace(rn.node_->id_);
    CHECK_MESSAGE(success, "A train iterator should visit a node only once.");
  }
}

// TODO(julian) Check for correctly omitted nodes

void check_train_iterator(train const& t, infrastructure const& infra) {
  check_element_uniqueness(t, infra);
  check_node_uniqueness(t, infra);
}

TEST_CASE("train iterator - follow") {
  infrastructure const infra(soro::test::SMALL_OPTS);
  timetable const tt(soro::test::FOLLOW_OPTS, infra);

  for (auto const& t : tt) {
    check_train_iterator(*t, infra);
  }
}