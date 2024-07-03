#include "soro/infrastructure/parsers/iss/get_brake_tables.h"

#include <cmath>
#include <algorithm>
#include <iterator>

#include "utl/erase_if.h"
#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/base/fp_precision.h"
#include "soro/base/soro_types.h"

#include "soro/utls/narrow.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/for_each.h"
#include "soro/utls/std_wrapper/is_sorted.h"
#include "soro/utls/std_wrapper/sort.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/brake_tables.h"
#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

#include "soro/rolling_stock/train_physics.h"

namespace soro::infra {

using namespace soro::rs;

struct full_entry {
  si::speed speed_{si::speed::invalid()};
  si::slope slope_{si::slope::invalid()};
  brake_path brake_path_{brake_path::invalid()};
  brake_type brake_type_{brake_type::invalid()};
  brake_weight_percentage percentage_{brake_weight_percentage::invalid()};
};

brake_table::lines::size_type get_brake_table_index(si::slope const slope) {
  auto const idx = static_cast<brake_table::lines::size_type>(
      std::round(std::tan(si::as_radian(slope)) * 1000.0));

  utls::sassert(idx < brake_table::line_count, "idx {} not in range {}", idx,
                brake_table::line_count);

  return static_cast<brake_table::lines::size_type>(idx);
}

void insert_into_tables(brake_tables& tables, full_entry const& entry) {
  auto& table = tables.types_[entry.brake_type_][entry.brake_path_];

  auto const line_idx = get_brake_table_index(entry.slope_);
  auto& line = table.lines_[line_idx];
  utls::sassert(!line.slope_.is_valid() || line.slope_ == entry.slope_,
                "idx calculation went wrong");
  line.slope_ = entry.slope_;

  auto const& entry_idx = brake_table::line::get_idx(entry.speed_);

  if (entry_idx >= line.size()) {
    line.entries_.resize(as_val(entry_idx) + 1);
  }

  auto const is_larger = std::all_of(
      std::begin(line), std::begin(line) + as_val(entry_idx),
      [&](auto&& e) { return e.is_nan() || e <= entry.percentage_; });
  auto const is_smaller = std::all_of(
      std::begin(line) + as_val(entry_idx), std::end(line),
      [&](auto&& e) { return e.is_nan() || e >= entry.percentage_; });
  auto const sorted = is_larger && is_smaller;

  if (!sorted) {
    uLOG(utl::warn) << "brake weight percentage value " << entry.percentage_
                    << " not sorted with " << entry.brake_type_
                    << " brake type, " << entry.brake_path_ << " brake path, "
                    << entry.slope_ << " slope and " << entry.speed_
                    << " speed";
  } else {
    line[entry_idx] = entry.percentage_;
  }
}

void finalize_brake_table_line(brake_table::line& line) {
  auto const is_nan = [](auto&& e) { return e.is_nan(); };
  auto const is_not_nan = [](auto&& e) { return !e.is_nan(); };

  auto const get_next_nan = [&line, &is_nan](auto&& from) {
    return std::find_if(from, std::end(line.entries_), is_nan);
  };
  auto const get_next_non_nan = [&line, &is_not_nan](auto&& from) {
    return std::find_if(from, std::end(line.entries_), is_not_nan);
  };

  // any nan entries are overwritten to the closest following non-nan value
  auto next_nan = get_next_nan(std::begin(line.entries_));
  auto next_non_nan = get_next_non_nan(next_nan);

  utls::sassert(std::all_of(next_nan, next_non_nan, is_nan), "overwriting");
  std::fill(next_nan, next_non_nan, *next_non_nan);

  while (next_nan != std::end(line.entries_)) {
    next_nan = get_next_nan(next_non_nan);
    next_non_nan = get_next_non_nan(next_nan);

    utls::sassert(std::all_of(next_nan, next_non_nan, is_nan), "overwriting");
    std::fill(next_nan, next_non_nan, *next_non_nan);
  }

  utls::ensure(
      utls::all_of(line.entries_, [](auto&& e) { return !e.is_nan(); }),
      "found nan");
  utls::ensure(utls::is_sorted(line.entries_), "found non sorted line");
}

void finalize_brake_table(brake_table& table) {
  utl::erase_if(table.lines_, [](auto& l) { return l.empty(); });
  utls::sort(table.lines_);
  utls::for_each(table.lines_, finalize_brake_table_line);
}

void finalize_brake_tables(brake_tables& result) {
  utls::for_each(result.types_, [](auto& paths) {
    utls::for_each(paths, [](auto& table) { finalize_brake_table(table); });
  });
}

brake_tables get_brake_tables(iss_files const& iss_files,
                              dictionaries const& dicts) {
  utl::scoped_timer const timer("parsing brake tables");

  brake_tables result;

  // initialize brake table
  result.types_.resize(
      utls::narrow<brake_type::value_t>(dicts.brake_type_.keys_.size()));
  utls::for_each(result.types_, [&](auto&& bt) {
    bt.resize(
        utls::narrow<brake_path::value_t>(dicts.brake_path_.keys_.size()));
  });

  for (auto const& core_xml : iss_files.core_data_files_) {
    auto const& brake_tables =
        core_xml.child(XML_ISS_DATA).child(CORE_DATA).child(BRAKE_TABLE_CORES);

    for (auto const& table : brake_tables.children(BRAKE_TABLE_CORE)) {
      auto const speed = si::from_km_h(
          utls::parse_fp<si::speed::precision>(table.child_value(SPEED)));

      utls::sassert(speed.is_multiple_of(brake_table::line::step_size),
                    "speed {} is not multiple of step size {}", speed,
                    brake_table::line::step_size);

      auto const path_val = table.child(BRAKE_PATH_TYPE).attribute(KEY).value();
      auto const path_key = parse_dictionary_key<brake_path_key>(path_val);
      auto const path = dicts.brake_path_.get_id(path_key);

      auto const type_val = table.child(BRAKETYPE).attribute(KEY).value();
      auto const type_key = parse_dictionary_key<brake_type_key>(type_val);
      auto const type = dicts.brake_type_.get_id(type_key);

      for (auto const entry : table.child(BRAKE_TABLES).children(BRAKE_TABLE)) {
        auto const slope_x = entry.child(SLOPE);
        auto const percentage_x = entry.child(MINIMUM_BRAKE_WEIGHT_PERCENTAGE);

        if (!slope_x || !percentage_x) {
          continue;
        }

        full_entry full_entry;

        full_entry.brake_type_ = type;
        full_entry.brake_path_ = path;
        full_entry.speed_ = speed;

        auto const parsed_slope =
            utls::parse_fp<si::slope::precision, utls::replace_comma::ON>(
                slope_x.child_value());
        full_entry.slope_ = si::from_per_mille_gradient(parsed_slope);

        utls::sassert(zero(parsed_slope - std::round(parsed_slope)),
                      "slope {} is not a multiple of step size 1,0",
                      parsed_slope);

        auto const percentage = brake_weight_percentage{
            utls::parse_fp<brake_weight_percentage::precision>(
                percentage_x.child_value())};
        full_entry.percentage_ = percentage;

        insert_into_tables(result, full_entry);
      }
    }
  }

  finalize_brake_tables(result);

  utls::ensures([&] {
    utls::ensure(result.types_.size() == as_val(dicts.brake_type_.size()),
                 "not all brake types accounted for");

    auto const ensure_line = [](brake_table::line const& line) {
      utls::for_each(line.entries_,
                     [](auto&& e) { utls::ensure(!e.is_nan(), "found nan"); });
    };

    auto const ensure_table = [&](brake_table const& table) {
      utls::for_each(table.lines_, ensure_line);
    };

    for (auto const& paths : result.types_) {
      utls::ensure(paths.size() == as_val(dicts.brake_path_.size()),
                   "not all brake paths accounted for");
      utls::for_each(paths, [&](auto&& table) { ensure_table(table); });
    }
  });

  return result;
}

}  // namespace soro::infra
