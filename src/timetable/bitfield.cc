#include "soro/timetable/bitfield.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

#include "utl/verify.h"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

#include "soro/utls/narrow.h"
#include "soro/utls/sassert.h"

namespace soro::tt {

bitfield::anchor_time absolute_time_to_anchor_time(absolute_time const at) {
  return sc::time_point_cast<bitfield::anchor_time::duration>(midnight(at));
}

std::size_t distance(bitfield::anchor_time const from,
                     bitfield::anchor_time const to) {
  utls::sassert(bitfield::valid(from), "Date from {} is not ok.", from);
  utls::sassert(bitfield::valid(to), "Date to {} is not ok.", to);
  utls::sassert(from <= to,
                "Call distance for year_month_date with ascending dates, "
                "got called with {} and {}",
                from, to);

  return static_cast<std::size_t>((to - from).count());
}

bitfield::iterator::idx_t get_end_idx(bitfield const* bf) {
  return static_cast<bitfield::iterator::idx_t>(
      distance(bf->first_date_, bf->last_date_) + 1);
}

std::size_t get_idx(bitfield::anchor_time const first_date,
                    bitfield::anchor_time const last_date,
                    bitfield::anchor_time const ymd) {
  utls::sassert(
      first_date <= ymd && ymd <= last_date,
      "Trying to update bitfield with date {} that is not in range [{}, {}].",
      ymd, first_date, last_date);

  return distance(first_date, ymd);
}

bitfield::anchor_time idx_to_date(bitfield::anchor_time const first_date,
                                  std::size_t const idx) {
  utls::sassert(bitfield::valid(first_date), "First date {} is not ok.",
                first_date);
  return {first_date + days(idx)};
}

bitfield::iterator::iterator(bitfield const* bitfield, idx_t const idx)
    : bitfield_{bitfield}, idx_{idx} {
  utls::expect(idx_ <= get_end_idx(bitfield), "got invalid idx");

  if (idx_ != get_end_idx(bitfield) &&
      !bitfield_->at(idx_to_date(bitfield_->first_date_, idx_))) {
    this->operator++();
  }

  utls::ensure(idx_ <= get_end_idx(bitfield), "created invalid idx");
}

bitfield::iterator& bitfield::iterator::operator++() {
  utls::expect(idx_ < get_end_idx(bitfield_), "incrementing end iterator");
  ++idx_;

  while (idx_ < get_end_idx(bitfield_) && !bitfield_->bitset_[idx_]) {
    ++idx_;
  }

  utls::ensure(idx_ <= get_end_idx(bitfield_), "created invalid iterator");
  utls::ensure(idx_ == get_end_idx(bitfield_) || bitfield_->bitset_[idx_],
               "pointing to non set day");

  return *this;
}

bitfield::iterator::value_type bitfield::iterator::operator*() const {
  utls::sassert(bitfield_->at(bitfield_->first_date_ + days{idx_}),
                "returning non set day");
  return bitfield_->first_date_ + days{idx_};
}

bitfield::iterator bitfield::begin() const {
  return bitfield::iterator{this, 0};
}

bitfield::iterator bitfield::end() const {
  return bitfield::iterator{this, get_end_idx(this)};
}

bitfield::iterator bitfield::from(absolute_time const t) const {
  auto const at = sc::time_point_cast<anchor_time::duration>(midnight(t));

  if (at <= first_date_) {
    return iterator{this, 0};
  } else {
    auto const dist = distance(first_date_, at);
    auto const max_dist = get_end_idx(this);

    return dist >= max_dist
               ? iterator{this, max_dist}
               : iterator{this, static_cast<bitfield::iterator::idx_t>(dist)};
  }
}

bitfield::iterator bitfield::to(absolute_time const t) const {
  auto const at = sc::time_point_cast<anchor_time::duration>(midnight(t));

  if (at >= last_date_) {
    return iterator{this, get_end_idx(this)};
  }

  if (at < first_date_) {
    return iterator{this, 0};
  }

  return iterator{this, static_cast<bitfield::iterator::idx_t>(
                            distance(first_date_, at) + 1)};
}

bool bitfield::ok() const noexcept {
  return valid(this->first_date_) && valid(this->last_date_);
}

std::vector<bitfield::anchor_time> bitfield::get_set_days() const {
  std::vector<bitfield::anchor_time> result;

  for (auto i = 0UL; i < BITSIZE; ++i) {
    if (this->bitset_.test(i)) {
      result.push_back(idx_to_date(this->first_date_, i));
    }
  }

  return result;
}

bool bitfield::at(anchor_time const ymd) const {
  if (first_date_ <= ymd && ymd <= last_date_) {
    return bitset_[get_idx(first_date_, last_date_, ymd)];
  }

  throw utl::fail(
      "Out of bounds access in bitfield. Trying to access with date {}, but "
      "got range [{}, {}]",
      ymd, first_date_, last_date_);
}

bool bitfield::operator[](anchor_time const ymd) const {
  return bitset_[get_idx(first_date_, last_date_, ymd)];
}

// returns:
// ymd < first: -1
// ymd in [first, last]: 0
// ymd > last: 1
int8_t in_range(bitfield::anchor_time const ymd,
                bitfield::anchor_time const first,
                bitfield::anchor_time const last) {
  if (ymd < first) {
    return -1;
  }

  if (ymd <= last) {
    return 0;
  }

  return 1;
}

void bitfield::set(bitfield::anchor_time const ymd, bool const new_value) {
  auto const pos = in_range(ymd, this->first_date_, this->last_date_);

  anchor_time const anchor{ymd};

  if (pos < 0) {
    this->bitset_ <<= distance(anchor, this->first_date_);
    this->first_date_ = ymd;
  } else if (pos > 0) {
    this->last_date_ = ymd;
  }  // else pos == 0, no changes to dates

  utls::sassert(
      distance(this->first_date_, this->last_date_) < BITSIZE,
      "Covering from start date {} to last date {} impossible with {} bits.",
      this->first_date_, this->last_date_, BITSIZE);

  bitset_.set(get_idx(first_date_, last_date_, ymd), new_value);
}

bool bitfield::equivalent(bitfield const& o) const noexcept {
  if (*this == o) {
    return true;
  }

  if (this->first_date_ < o.first_date_) {
    auto copy = o;
    copy.bitset_ >> distance(this->first_date_, o.first_date_);
    return *this == copy;
  } else {
    auto copy = *this;
    copy.bitset_ >> distance(o.first_date_, this->first_date_);
    return copy == o;
  }
}

bitfield operator|(bitfield const& lhs, bitfield const& rhs) noexcept {
  auto copy = lhs;
  copy |= rhs;
  return copy;
}

bitfield bitfield::operator|=(bitfield const& o) noexcept {
  utls::sassert(distance(std::min(this->first_date_, o.first_date_),
                         std::max(this->last_date_, o.last_date_)) < BITSIZE,
                "Covering more days than possible with only {} bits.", BITSIZE);

  if (this->first_date_ < o.first_date_) {
    auto const c = o.bitset_ << distance(this->first_date_, o.first_date_);
    this->bitset_ |= c;
  } else {
    this->bitset_ <<= distance(o.first_date_, this->first_date_);
    this->bitset_ |= o.bitset_;
    this->first_date_ = o.first_date_;
  }

  this->last_date_ = std::max(this->last_date_, o.last_date_);

  return *this;
}

soro::size_t bitfield::count() const noexcept {
  return utls::narrow<soro::size_t>(this->bitset_.count());
}

absolute_time bitfield::first_set_date() const noexcept {
  utls::expect(bitset_.any(), "no bits set in bitfield, procedure will fail");

  std::size_t first_set_idx = 0;

  while (!bitset_.test(first_set_idx)) ++first_set_idx;

  return absolute_time{idx_to_date(first_date_, first_set_idx)};
}

absolute_time bitfield::last_set_date() const noexcept {
  utls::expect(bitset_.any(), "no bits set in bitfield, procedure will fail");

  std::size_t last_set_idx = distance(first_date_, last_date_);

  while (!bitset_.test(last_set_idx)) --last_set_idx;

  return absolute_time{idx_to_date(first_date_, last_set_idx)};
}

bitfield make_bitfield(bitfield::anchor_time const first_date,
                       bitfield::anchor_time const last_date,
                       const char* const bitmask) {
  auto const bitmask_length = distance(first_date, last_date) + 1;

  utls::sasserts([&]() {
    utls::sassert(first_date <= last_date, "Last date {} before first date {}.",
                  last_date, first_date);

    auto const strlen_length = strlen(bitmask);
    utls::sassert(strlen_length == distance(first_date, last_date) + 1,
                  "Distance between first date {} and last date {} is {}, but "
                  "bit_string has length of {}.",
                  first_date, last_date, distance(first_date, last_date),
                  strlen_length);
  });

  std::array<char, bitfield::BITSIZE> buf{};
  std::fill(std::begin(buf), std::end(buf), '0');

  std::copy(bitmask, bitmask + bitmask_length,
            std::end(buf) - static_cast<std::ptrdiff_t>(bitmask_length));
  std::reverse(std::end(buf) - static_cast<std::ptrdiff_t>(bitmask_length),
               std::end(buf));

  std::string_view const bits_view{std::begin(buf), std::end(buf)};

  return {.first_date_ = first_date,
          .last_date_ = last_date,
          .bitset_ = bitfield::bitset{bits_view}};
}

}  // namespace soro::tt
