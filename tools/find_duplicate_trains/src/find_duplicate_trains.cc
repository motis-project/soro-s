#include <fstream>
#include <iostream>

#include "utl/cmd_line_parser.h"
#include "utl/concat.h"
#include "utl/parallel_for.h"
#include "utl/timer.h"

#include "soro/infrastructure/graph/type_set.h"
#include "soro/infrastructure/infrastructure.h"

#include "soro/rolling_stock/train_series.h"

#include "soro/runtime/euler_runtime.h"

#include "soro/timetable/timetable.h"
#include "soro/timetable/timetable_options.h"

using namespace soro;
using namespace soro::tt;
using namespace soro::infra;
using namespace utl;

namespace fs = std::filesystem;

struct config {
  cmd_line_flag<fs::path, required, UTL_LONG("--infra_path"),
                UTL_DESC("path to the infrastructure directory")>
      infra_path_;

  cmd_line_flag<fs::path, required, UTL_LONG("--timetable_path"),
                UTL_DESC("path to the timetable directory")>
      timetable_path_;

  cmd_line_flag<fs::path, required, UTL_LONG("--output_path"),
                UTL_DESC("path to the output file")>
      output_path_;

  bool valid_paths() const {
    return fs::exists(infra_path_.val()) &&
           fs::is_directory(infra_path_.val()) &&
           fs::exists(timetable_path_.val()) &&
           fs::is_directory(timetable_path_.val());
  }
};

int failed_parsing() {
  std::cout << "please set a valid infrastructure directory\n\n";
  std::cout << description<config>();
  std::cout << '\n';

  return 1;
}

struct manually_removed {
  train::number number_;
  std::string reason_;
};

struct sr_usage {
  CISTA_COMPARABLE()

  absolute_time start_;
  train::id train_;
};

struct double_pair {
  CISTA_COMPARABLE()

  train::id train1_;
  train::id train2_;
};

std::set<train::id> find_duplicates(soro::vector<train> const& trains,
                                    infrastructure const& infra) {
  utl::scoped_timer const timer("find duplicates");

  soro::vector_map<station_route::id, std::vector<sr_usage>> sr_usages(
      infra->station_routes_.size());

  for (auto const& train : trains) {
    for (auto const midnight : train.departures()) {
      for (auto const& sp : train.sequence_points_) {
        if (auto const arr = sp.absolute_arrival(midnight); arr.has_value()) {
          sr_usages[sp.station_route_].emplace_back(sr_usage{*arr, train.id_});
        }
      }
    }
  }

  utl::parallel_for(sr_usages, [](auto&& sr_usage) {
    std::sort(std::begin(sr_usage), std::end(sr_usage));
  });

  std::map<double_pair, uint32_t> double_pairs;

  for (auto const& sr_usage : sr_usages) {
    for (auto const [u1, u2] : utl::pairwise(sr_usage)) {
      if (u1.start_ == u2.start_ && u1.train_ != u2.train_) {
        double_pair const dp{.train1_ = u1.train_, .train2_ = u2.train_};
        ++double_pairs[dp];
      }
    }
  }

  // order -> double pair count
  std::map<uint32_t, uint32_t> double_pair_stats;

  for (auto const& [dp, order] : double_pairs) {
    ++double_pair_stats[order];
  }

  uLOG(utl::info) << "total double pairs: " << double_pairs.size();
  uLOG(utl::info) << "double pair stats:";
  for (auto const& [order, count] : double_pair_stats) {
    uLOG(utl::info) << "dp order " << order << " count: " << count;
  }

  std::set<train::id> duplicates;

  for (auto const& [double_pair, order] : double_pairs) {
    if (duplicates.contains(double_pair.train1_) ||
        duplicates.contains(double_pair.train2_)) {
      continue;
    }

    auto const shorter = trains[double_pair.train1_].path_.size() <
                                 trains[double_pair.train2_].path_.size()
                             ? double_pair.train1_
                             : double_pair.train2_;

    duplicates.insert(shorter);
  }

  return duplicates;
}

void write_out_duplicates(std::set<train::id> const& duplicates,
                          timetable const& tt,
                          std::filesystem::path const& out_path) {
  std::ofstream out(out_path);

  for (auto const id : duplicates) {
    auto const& train = tt->trains_[id];
    out << train.number_.main_ << "," << train.number_.sub_ << ",duplicate\n";
  }

  out.close();
}

void write_out_manually_removed(std::vector<manually_removed> const& removed,
                                std::filesystem::path const& out_path) {
  std::ofstream out(out_path);

  for (auto const r : removed) {
    out << r.number_.main_ << "," << r.number_.sub_ << "," << r.reason_ << "\n";
  }

  out.close();
}

bool can_calculate_on_time(train const& train, infrastructure const& infra) {
  auto const timestamps =
      runtime::runtime_calculation(train, infra, {type::HALT});

  utls::sassert(train.total_halts() == timestamps.times_.size(),
                "timestamps size must be equal to total halts");

  std::size_t timestamp_idx = 0;

  for (auto const& sp : train.sequence_points_) {
    if (!sp.is_halt()) continue;

    if (*sp.arrival_ >= timestamps.times_[timestamp_idx].arrival_) {
      return false;
    }
  }

  return true;
}

std::vector<manually_removed> get_not_on_time_trains(
    soro::vector<train> const& trains, infrastructure const& infra) {
  std::vector<manually_removed> removed;

  for (auto const& train : trains) {
    if (!can_calculate_on_time(train, infra)) {
      removed.emplace_back(
          manually_removed{.number_ = train.number_, .reason_ = "not_on_time"});
    }
  }

  return removed;
}

int main(int argc, char const** argv) {
  config c;

  std::cout << "\n\tDuplicate Train Finder\n\n";
  try {
    c = parse<struct config>(argc, argv);
  } catch (...) {
    return failed_parsing();
  }

  if (!c.valid_paths()) {
    return failed_parsing();
  }

  auto opts = make_infra_opts(c.infra_path_.val(), "");
  opts.layout_ = false;
  opts.interlocking_ = true;
  opts.exclusions_ = false;
  opts.exclusion_sets_ = false;

  auto const tt_opts = make_timetable_opts(c.timetable_path_.val());

  infrastructure const infra(opts);
  timetable const tt(tt_opts, infra);

  auto removed = get_not_on_time_trains(tt->trains_, infra);

  utl::concat(removed, get_duplicates(tt->trains_, infra));

  write_out_manually_removed(removed, c.output_path_.val());

  //  auto const duplicates = find_duplicates(tt->trains_, infra);
  //  write_out_duplicates(duplicates, tt, c.output_path_.val());
}
