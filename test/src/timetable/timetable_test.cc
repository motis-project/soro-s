#include "doctest/doctest.h"

#include "soro/utls/coroutine/coro_map.h"

#include "soro/infrastructure/infrastructure.h"
#include "soro/infrastructure/path/length.h"
#include "soro/timetable/timetable.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::utls;
using namespace soro::infra;

namespace soro::tt::test {

void check_arrival_departures(timetable const& tt) {
  for (auto const& train : tt) {
    CHECK_MESSAGE((train->stop_times_.front().arrival_ == INVALID_TIME),
                  "First arrival must be invalid");
    CHECK_MESSAGE((train->stop_times_.front().departure_ != INVALID_TIME),
                  "First departure must be valid");
    CHECK_MESSAGE((train->stop_times_.back().arrival_ != INVALID_TIME),
                  "Last arrival must be valid");
    CHECK_MESSAGE((train->stop_times_.back().departure_ == INVALID_TIME),
                  "Last departure must be invalid");

    std::size_t idx = 0;
    for (auto const [arr, dep, mst] : train->stop_times_) {
      if (idx == 0 || idx == train->stop_times_.size() - 1) {
        continue;
      }

      ++idx;
      bool const both_invalid = arr == INVALID_TIME && dep == INVALID_TIME;
      bool const both_valid = arr != INVALID_TIME && dep != INVALID_TIME;
      bool const either_or = both_valid || both_invalid;
      CHECK_MESSAGE(either_or, "Arr/Dep must both be valid or invalid");
    }
  }
}

void check_no_invalids(timetable const& tt) {
  for (auto const& train : tt) {
    for (auto const& ir_id : train->path_) {
      CHECK_MESSAGE(interlocking_route::valid(ir_id),
                    "Signal station route is nullptr!");
    }
  }
}

void check_unique_ascending_ids(timetable const& tt) {
  for (auto const [tr1, tr2] : utl::pairwise(tt)) {
    CHECK_MESSAGE((tr1->id_ + 1 == tr2->id_),
                  "IDs must be consecutive and ascending!");
  }

  if (!tt->trains_.empty()) {
    CHECK_MESSAGE((tt->trains_.back()->id_ == tt->trains_.size() - 1),
                  "Last ID must be number of train runs - 1!");
  }
}

void check_timetable(timetable const& tt) {
  check_unique_ascending_ids(tt);
  check_arrival_departures(tt);
  check_no_invalids(tt);
}

void check_train_path_lengths(timetable const& tt,
                              infrastructure const& infra) {
  for (auto const& train : tt) {
    auto const e1 = get_path_length_from_elements(utls::coro_map(
        train->iterate(infra), [](auto&& rn) { return rn.node_; }));

    CHECK_MESSAGE((train->length_ == e1),
                  "Different lengths from the two length calculation funs");
  }
}

TEST_CASE("timetable test") {
  for (auto const& scenario : soro::test::get_timetable_scenarios()) {
    check_timetable(scenario.timetable_);
    check_train_path_lengths(scenario.timetable_, scenario.infra_);
  }
}

}  // namespace soro::tt::test
