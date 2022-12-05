#include "soro/timetable/bitfield.h"

#include "utl/verify.h"

#include "soro/utls/sassert.h"

namespace soro::tt {

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

std::size_t distance(date::year_month_day const from,
                     date::year_month_day const to) {
  utls::sassert(from.ok(), "Date from {} is not ok.", from);
  utls::sassert(to.ok(), "Date to {} is not ok.", to);
  utls::sassert(from <= to,
                "Call distance for year_month_date with ascending dates, "
                "got called with {} and {}",
                from, to);

  return distance(bitfield::anchor_time{from}, bitfield::anchor_time{to});
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

date::year_month_day idx_to_date(bitfield::anchor_time const first_date,
                                 std::size_t const idx) {
  utls::sassert(bitfield::valid(first_date), "First date {} is not ok.",
                first_date);
  return {first_date + days(idx)};
}

bool bitfield::ok() const noexcept {
  return valid(this->first_date_) && valid(this->last_date_);
}

std::vector<date::year_month_day> bitfield::get_set_days() const {
  std::vector<date::year_month_day> result;

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

void bitfield::set(date::year_month_day const ymd, bool const new_value) {
  auto const pos = in_range(ymd, this->first_date_, this->last_date_);

  anchor_time const ymd_anchor{ymd};

  if (pos < 0) {
    this->bitset_ <<= distance(ymd_anchor, this->first_date_);
    this->first_date_ = ymd;
  } else if (pos > 0) {
    this->last_date_ = ymd;
  }  // else pos == 0, no changes to dates

  utls::sassert(
      distance(this->first_date_, this->last_date_) < BITSIZE,
      "Covering from start date {} to last date {} impossible with BITSIZE {}.",
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

bitfield::anchor_time bitfield::end() const noexcept {
  return {static_cast<date::sys_days>(this->first_date_) + date::days{BITSIZE}};
}

std::size_t bitfield::count() const noexcept { return this->bitset_.count(); }

bitfield make_bitfield(date::year_month_day const first_date,
                       date::year_month_day const last_date,
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
            std::end(buf) - static_cast<ssize_t>(bitmask_length));
  std::reverse(std::end(buf) - static_cast<ssize_t>(bitmask_length),
               std::end(buf));

  std::string_view const bits_view{std::begin(buf), std::end(buf)};

  return {.first_date_ = first_date,
          .last_date_ = last_date,
          .bitset_ = bitfield::bitset{bits_view}};
}

}  // namespace soro::tt
