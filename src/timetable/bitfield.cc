#include "soro/timetable/bitfield.h"

#include "utl/verify.h"

#include "soro/utls/sassert.h"

namespace soro::tt {

std::size_t distance(date::year_month_day const from,
                     date::year_month_day const to) {
  utls::sassert(from.ok(), "Date from {} is not ok.", from);
  utls::sassert(to.ok(), "Date to {} is not ok.", to);
  utls::sassert(from <= to,
                "Call distance for year_month_date with ascending dates, "
                "got called with {} and {}",
                from, to);
  return static_cast<std::size_t>(
      (static_cast<date::sys_days>(to) - static_cast<date::sys_days>(from))
          .count());
}

std::size_t get_idx(date::year_month_day const first_date,
                    date::year_month_day const last_date,
                    date::year_month_day const ymd) {
  utls::sassert(
      first_date <= ymd && ymd <= last_date,
      "Trying to update bitfield with date {} that is not in range [{}, {}].",
      ymd, first_date, last_date);

  return distance(first_date, ymd);
}

date::year_month_day idx_to_date(date::year_month_day first_date,
                                 std::size_t const idx) {
  utls::sassert(first_date.ok(), "First date {} is not ok.", first_date);

  auto const first_days = static_cast<date::sys_days>(first_date);
  auto const new_days = first_days + date::days(idx);
  return {new_days};
}

bitfield::bitfield(date::year_month_day const start,
                   date::year_month_day const end, std::string bit_string)
    : first_date_{start}, last_date_{end} {
  utls::sassert(
      first_date_ <= last_date_,
      "Trying to construct bitfield with last date {} before first date {}.",
      last_date_, first_date_);
  utls::sassert(distance(first_date_, last_date_) < BITSIZE,
                "Trying to cover {} days with only {} bits.",
                distance(first_date_, last_date_), BITSIZE);

  std::string bits(bitfield::BITSIZE, '0');

  std::reverse(std::begin(bit_string), std::end(bit_string));
  std::copy(std::begin(bit_string), std::end(bit_string),
            std::end(bits) - static_cast<ssize_t>(bit_string.size()));

  this->bitset_.set(bits);
}

bool bitfield::ok() const noexcept {
  return this->first_date_.ok() && this->last_date_.ok();
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

bool bitfield::at(date::year_month_day const ymd) const {
  if (first_date_ <= ymd && ymd <= last_date_) {
    return bitset_[get_idx(first_date_, last_date_, ymd)];
  }

  throw utl::fail(
      "Out of bounds access in bitfield. Trying to access with date {}, but "
      "got range [{}, {}]",
      ymd, first_date_, last_date_);
}

bool bitfield::operator[](date::year_month_day const ymd) const noexcept {
  return bitset_[get_idx(first_date_, last_date_, ymd)];
}

void bitfield::set(date::year_month_day const ymd, bool const new_value) {
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

}  // namespace soro::tt
