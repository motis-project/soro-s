#include "doctest/doctest.h"

#include <utility>

#include "date/date.h"

#include "utl/pairwise.h"

#include "soro/base/time.h"

#include "soro/utls/serializable.h"
#include "soro/utls/std_wrapper/contains.h"
#include "test/utls/has_cycle.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/timetable/interval.h"
#include "soro/timetable/timetable.h"

#include "soro/ordering/graph.h"

#include "test/file_paths.h"

#include "test/ordering/all_train_edges_exist.h"
#include "test/ordering/check_train_edge_transitivity.h"
#include "test/ordering/has_exclusion_paths.h"

namespace soro::ordering::test {

using namespace soro::tt;
using namespace soro::infra;
using namespace soro::test;

void check_ordering_graph(graph const& og, infrastructure const& infra,
                          timetable const& tt) {
  // no cycles allowed in the ordering graph
  CHECK(!has_cycle(og));

  // all exclusion paths have to exist
  CHECK(has_exclusion_paths(og, infra));

  // all train edges have to exist
  CHECK(all_train_edges_exist(og));

  // train edges can be transitive in a number of select cases
  // if we do not have one of those cases the transitivity indicates a bug
  CHECK(check_train_edge_transitivity(og, infra, tt));
}

TEST_SUITE("ordering graph") {
  TEST_CASE("ordering follow") {
    auto opts = soro::test::SMALL_OPTS;
    auto tt_opts = soro::test::FOLLOW_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);

    // we have two trains, with one trip each
    CHECK_EQ(tt->trains_.size(), 2);
    CHECK_EQ(tt->trains_.front().trip_count(), 1);
    CHECK_EQ(tt->trains_.back().trip_count(), 1);

    auto const& [earlier, later] =
        tt->trains_.front().start_time_ <= tt->trains_.back().end_time_
            ? std::pair{tt->trains_.front(), tt->trains_.back()}
            : std::pair{tt->trains_.back(), tt->trains_.front()};

    // both take the same path
    CHECK_EQ(earlier.path_, later.path_);

    graph const og(infra, tt);

    check_ordering_graph(og, infra, tt);

    // the ordering graph has a node for every {train, interlocking route} pair
    CHECK_EQ(earlier.path_.size() + later.path_.size(), og.nodes_.size());

    auto const earlier_trip = og.trips_.front().train_id_ == earlier.id_
                                  ? og.trips_.front()
                                  : og.trips_.back();

    auto const later_trip = og.trips_.back().train_id_ == later.id_
                                ? og.trips_.back()
                                : og.trips_.front();

    // check train edges
    for (auto const [from, to] : utl::pairwise(earlier_trip.nodes(og))) {
      CHECK(utls::contains(from.out(og), to.get_id(og)));
      CHECK(utls::contains(to.in(og), from.get_id(og)));
    }

    // check ordering edges
    for (auto idx = 0U; idx < earlier_trip.nodes(og).size(); ++idx) {
      auto const& from = earlier_trip.nodes(og)[idx];
      auto const& to = later_trip.nodes(og)[idx];

      CHECK(utls::contains(from.out(og), to.get_id(og)));
      CHECK(utls::contains(to.in(og), from.get_id(og)));
    }
  }

  TEST_CASE("ordering cross") {
    auto opts = soro::test::SMALL_OPTS;
    auto tt_opts = soro::test::CROSS_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);
    graph const og(infra, tt);

    check_ordering_graph(og, infra, tt);
  }

  TEST_CASE("ordering de_kss 2h") {
    using namespace date;

    interval const inter{
        .start_ = ymd_to_abs(2021_y / November / 13) + hours{7},
        .end_ = ymd_to_abs(2021_y / November / 13) + hours{9}};

    ordering::graph::filter const filter{
        .interval_ = inter, .include_trains_ = {}, .exclude_trains_ = {}};

    auto const infra = utls::try_deserializing<infrastructure>(
        "infra.raw", soro::test::DE_ISS_OPTS);

    auto const tt = utls::try_deserializing<timetable>(
        "timetable.raw", soro::test::DE_KSS_OPTS, infra);

    graph const og(infra, tt, filter);

    check_ordering_graph(og, infra, tt);
  }

  TEST_CASE("ordering de_kss 24h") {
    using namespace date;

    auto opts = soro::test::DE_ISS_OPTS;
    auto tt_opts = soro::test::DE_KSS_OPTS;

    opts.exclusions_ = true;
    opts.interlocking_ = true;
    opts.layout_ = false;

    infrastructure const infra(opts);
    timetable const tt(tt_opts, infra);

    interval const inter{
        .start_ = ymd_to_abs(2021_y / November / 13),
        .end_ = ymd_to_abs(2021_y / November / 13) + hours{24}};

    ordering::graph::filter const filter{
        .interval_ = inter, .include_trains_ = {}, .exclude_trains_ = {}};

    graph const og(infra, tt, filter);

    check_ordering_graph(og, infra, tt);
  }
}

}  // namespace soro::ordering::test