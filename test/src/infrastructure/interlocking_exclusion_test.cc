#include "doctest/doctest.h"

#include "test/file_paths.h"

#include "soro/infrastructure/interlocking/exclusion2.h"

using namespace soro;
using namespace soro::infra;

TEST_SUITE("interlocking exclusion") {
  TEST_CASE("t") {
    auto opts = test::DE_ISS_OPTS;
    //    auto opts = test::SMALL_OPTS;
    opts.determine_conflicts_ = false;
    opts.determine_interlocking_ = true;
    opts.determine_layout_ = false;

    infrastructure infra(opts);

    auto const sets = get_interlocking_exclusion_sets(infra);

    std::size_t non_empty = 0;
    std::size_t diff = 0;
    std::vector<uint32_t> diffs;
    for (auto const& set : sets) {
      if (set.empty()) {
        continue;
      }

      ++non_empty;
      diff += set.last_ - set.first_;
      diffs.emplace_back(set.last_ - set.first_);
    }

    std::sort(std::begin(diffs), std::end(diffs));

    std::cout << "exclusion sets\n";
    std::cout << "non empty: " << non_empty << '\n';
    std::cout << "avg diff: " << diff / non_empty << '\n';
    std::cout << "25%: " << diffs[diffs.size() * 0.25] << '\n';
    std::cout << "50%: " << diffs[diffs.size() * 0.5] << '\n';
    std::cout << "75%: " << diffs[diffs.size() * 0.75] << '\n';
    std::cout << "90%: " << diffs[diffs.size() * 0.9] << '\n';
    std::cout << "99%: " << diffs[diffs.size() * 0.99] << '\n';
  }
}
