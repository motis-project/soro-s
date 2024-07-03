#include "soro/infrastructure/brake_tables.h"

#include <cmath>
#include <algorithm>
#include <iterator>

#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/distance.h"
#include "soro/utls/std_wrapper/lower_bound.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"

#include "soro/rolling_stock/train_physics.h"

namespace soro::infra {

using namespace soro::rs;

brake_table::line::index brake_table::line::get_idx(
    rs::brake_weight_percentage const percentage) const {
  auto it = utls::lower_bound(entries_, percentage);
  it =
      std::find_if(it, std::end(entries_), [&](auto&& e) { return e != *it; }) -
      1;

  auto const is_end = it == std::end(entries_);
  auto const too_big = !is_end && (*it) > percentage;
  auto const is_begin = it == std::begin(entries_);

  if ((is_end || too_big) && !is_begin) --it;

  if (*it > percentage) {
    throw utl::fail(
        "insufficient brake weight percentage. given {}, required {}",
        percentage, *it);
  }

  return index{utls::distance<index::value_t>(std::begin(entries_), it)};
}

brake_table::line::index brake_table::line::get_idx(si::speed const speed) {
  utls::expect(speed.is_multiple_of(step_size),
               "speed {} not multiple of step size: {}", speed, step_size);

  auto const idx = static_cast<index::value_t>(
      std::round(si::as_precision(speed / step_size)));

  return index{idx};
}

si::speed brake_table::line::get_speed(index const idx) {
  return brake_table::line::step_size * as_val(idx);
}

si::speed brake_table::line::get_speed(
    rs::brake_weight_percentage const percentage) const {
  return get_speed(get_idx(percentage));
}

rs::brake_weight_percentage brake_table::line::get_percentage(
    index const idx) const {
  utls::ensure(idx < size(), "idx {} not in range {}", idx, size());
  return entries_[as_val(idx)];
}

rs::brake_weight_percentage brake_table::line::get_percentage(
    si::speed const speed) const {
  return get_percentage(get_idx(speed));
}

rs::brake_weight_percentage const& brake_table::line::operator[](
    index const idx) const {
  utls::ensure(idx < size(), "idx {} not in range {}", idx, size());
  return entries_[as_val(idx)];
}

rs::brake_weight_percentage& brake_table::line::operator[](index const idx) {
  utls::ensure(idx < size(), "idx {} not in range {}", idx, size());
  return entries_[as_val(idx)];
}

brake_table::line::entries::const_iterator brake_table::line::begin() const {
  return std::begin(entries_);
}

brake_table::line::entries::const_iterator brake_table::line::end() const {
  return std::end(entries_);
}

brake_table::line::length brake_table::line::size() const {
  return utls::narrow<length>(entries_.size());
}

bool brake_table::line::empty() const { return entries_.empty(); }

bool brake_table::line::has_entry(index const idx) const {
  return as_val(idx) < size();
}

bool brake_table::line::operator<(line const& other) const {
  return slope_ < other.slope_;
}

bool brake_table::empty() const { return lines_.empty(); }

si::speed brake_table::get_max_speed(
    si::slope const slope,
    rs::brake_weight_percentage const train_percentage) const {
  if (lines_.empty()) throw utl::fail("no lines found in brake table");

  auto const it = utls::lower_bound(
      lines_, slope, [](auto&& line, auto&& s) { return line.slope_ < s; });

  if (it == std::end(lines_)) {
    return std::prev(it)->get_speed(train_percentage);
  }

  if (it->slope_ == slope) {
    return it->get_speed(train_percentage);
  }

  return interpolate(it, slope, train_percentage);
}

si::speed brake_table::interpolate(
    lines::const_iterator const it, si::slope const slope,
    rs::brake_weight_percentage const train_percentage) const {
  auto idx = it->get_idx(train_percentage);

  auto const is_first_line = it == std::begin(lines_);
  auto const no_prev = !is_first_line && std::prev(it)->has_entry(idx);
  if (is_first_line || no_prev) return it->get_speed(train_percentage);

  // ok, since it is not begin
  auto const prev = std::prev(it);

  // linear interpolation of brake weight percentage depending on the slopes
  auto const factor =
      si::as_precision((slope - prev->slope_) / (it->slope_ - prev->slope_));

  auto const interpol = [&it, &prev, &factor](line::index const i) {
    return prev->get_percentage(i) +
           factor * (it->get_percentage(i) - prev->get_percentage(i));
  };

  auto const interpolated = interpol(idx);

  if (interpolated <= train_percentage) {
    while (idx != it->size() && idx != prev->size() &&
           interpol(idx) > train_percentage) {
      ++idx;
    }
  } else {  // interpolated > train_percentage
    while (idx != it->size() && idx != prev->size() &&
           interpol(idx) <= train_percentage) {
      --idx;
    }
  }

  utls::sassert(prev->get_speed(idx) == it->get_speed(idx),
                "two brake tables differ in speed for the idx {}", idx);

  return prev->get_speed(idx);
}

si::speed brake_tables::get_max_speed(
    rs::brake_type const brake_type, brake_path const brake_path,
    si::slope const slope,
    rs::brake_weight_percentage const train_percentage) const {
  auto const& table = types_[brake_type][brake_path];
  return table.get_max_speed(slope, train_percentage);
}

}  // namespace soro::infra