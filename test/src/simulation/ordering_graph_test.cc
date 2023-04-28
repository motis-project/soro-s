#include "doctest/doctest.h"

#include <queue>
#include <stack>

#include "soro/base/time.h"

#include "soro/simulation/get_cycle.h"
#include "soro/simulation/ordering_graph.h"

#include "soro/utls/graph/has_cycle.h"
#include "soro/utls/std_wrapper/contains.h"

#include "test/file_paths.h"

namespace soro::simulation::test {

using namespace soro::tt;
using namespace soro::infra;

bool has_cycle(ordering_graph const& og) {
  return utls::has_cycle(
      og.nodes_, [](auto&& nodes, auto&& id) { return nodes[id].out_; });
}

void check_ordering_graph(ordering_graph const& og) { CHECK(!has_cycle(og)); }

TEST_CASE("ordering graph, follow") {
  auto opts = soro::test::SMALL_OPTS;
  auto tt_opts = soro::test::FOLLOW_OPTS;

  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.exclusion_graph_ = false;
  opts.layout_ = false;

  infrastructure const infra(opts);

  timetable const tt(tt_opts, infra);

  // we have two trains, with one trip each
  CHECK_EQ(tt->trains_.size(), 2);
  CHECK_EQ(tt->trains_.front().trip_count(), 1);
  CHECK_EQ(tt->trains_.back().trip_count(), 1);

  auto const& [earlier, later] =
      tt->trains_.front().first_departure() <=
              tt->trains_.back().first_departure()
          ? std::pair{tt->trains_.front(), tt->trains_.back()}
          : std::pair{tt->trains_.back(), tt->trains_.front()};

  // both take the same path
  CHECK_EQ(earlier.path_, later.path_);

  ordering_graph const og(infra, tt);

  check_ordering_graph(og);

  // the ordering graph has a node for every {train, interlocking route} pair
  CHECK_EQ(earlier.path_.size() + later.path_.size(), og.nodes_.size());

  auto const earlier_trip = earlier.trips().front();
  auto const later_trip = later.trips().front();

  auto const earlier_nodes = og.trip_nodes(earlier_trip);
  auto const later_nodes = og.trip_nodes(later_trip);

  // check train edges
  for (auto const [from, to] : utl::pairwise(earlier_nodes)) {
    CHECK(utls::contains(from.out_, to.id_));
    CHECK(utls::contains(to.in_, from.id_));
  }

  // check ordering edges
  for (auto idx = 0U; idx < earlier_nodes.size(); ++idx) {
    auto const& from = earlier_nodes[idx];
    auto const& to = later_nodes[idx];

    CHECK(utls::contains(from.out_, to.id_));
    CHECK(utls::contains(to.in_, from.id_));
  }
}

TEST_CASE("ordering graph, cross") {
  auto opts = soro::test::SMALL_OPTS;
  auto tt_opts = soro::test::CROSS_OPTS;

  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.exclusion_graph_ = false;
  opts.layout_ = false;

  infrastructure const infra(opts);

  timetable const tt(tt_opts, infra);

  ordering_graph const og(infra, tt);

  check_ordering_graph(og);
}

TEST_CASE("de_kss graph") {
  auto opts = soro::test::DE_ISS_OPTS;
  auto tt_opts = soro::test::DE_KSS_OPTS;

  opts.exclusions_ = true;
  opts.interlocking_ = true;
  opts.exclusion_graph_ = false;
  opts.layout_ = false;

  infrastructure const infra(opts);
  timetable const tt(tt_opts, infra);

  interval const inter{.start_ = rep_to_absolute_time(1636786800),
                       .end_ = rep_to_absolute_time(1636794000)};
  ordering_graph const og(infra, tt, {.interval_ = inter});

  auto const cycle = get_cycle(og);
  if (cycle.has_value()) {
    std::cout << "cycle: ";
    for (auto const& id : *cycle) {
      std::cout << id << " ";
    }
    std::cout << "\n";
  }
  //  check_ordering_graph(og);
}

}  // namespace soro::simulation::test