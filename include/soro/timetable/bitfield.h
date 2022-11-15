#pragma once

#include "cista/containers/bitset.h"
#include "date/date.h"

#include "soro/utls/sassert.h"

namespace soro::tt {

inline std::size_t distance(date::year_month_day const from,
                            date::year_month_day const to) {
  utls::sassert(from.ok(), "Date from {} is not ok.", from);
  utls::sassert(to.ok(), "Date to {} is not ok.", to);
  utls::sassert(from <= to,
                "Call distance for year_month_date with ascending dates, "
                "got called with {} and {}",
                from, to);
  return (static_cast<date::sys_days>(to) - static_cast<date::sys_days>(from))
      .count();
}

inline std::size_t get_idx(date::year_month_day const first_date,
                           date::year_month_day const last_date,
                           date::year_month_day const ymd) {
  utls::sassert(
      first_date <= ymd && ymd <= last_date,
      "Trying to update bitfield with date {} that is not in range [{}, {}].",
      ymd, first_date, last_date);

  return distance(first_date, ymd);
}

inline date::year_month_day idx_to_date(date::year_month_day first_date,
                                        std::size_t const idx) {
  utls::sassert(first_date.ok(), "First date {} is not ok.", first_date);

  auto const first_days = static_cast<date::sys_days>(first_date);
  auto const new_days = first_days + date::days(idx);
  return {new_days};
}

struct bitfield {
  static constexpr std::size_t BITSIZE = 512;

  bitfield() = default;
  bitfield(date::year_month_day const start, date::year_month_day const end,
           std::string const& bitfield)
      : first_date_{start}, last_date_{end}, bitfield_(bitfield) {
    utls::sassert(
        first_date_ < last_date_,
        "Trying to construct bitfield with last date {} before first date {}.",
        last_date_, first_date_);
    utls::sassert(distance(first_date_, last_date_) < BITSIZE,
                  "Trying to cover {} days with only {} bits.",
                  distance(first_date_, last_date_), BITSIZE);
  }

  std::vector<date::year_month_day> get_set_days() const {
    std::vector<date::year_month_day> result;

    for (auto i = 0UL; i < BITSIZE; ++i) {
      if (this->bitfield_.test(i)) {
        result.push_back(idx_to_date(this->first_date_, i));
      }
    }

    return result;
  }

  bool operator[](date::year_month_day const ymd) const {
    return bitfield_[get_idx(first_date_, last_date_, ymd)];
  }

  void set(date::year_month_day const ymd, bool const new_value) {
    bitfield_.set(get_idx(first_date_, last_date_, ymd), new_value);
  }

  bool operator==(bitfield const& o) const noexcept {
    if (this->first_date_ == o.first_date_) {
      return this->bitfield_ == o.bitfield_;
    }

    return false;
  }

  friend bitfield operator|(bitfield const& lhs, bitfield const& rhs) noexcept {
    auto copy = lhs;
    copy |= rhs;
    return copy;
  }

  bitfield operator|=(bitfield const& o) noexcept {
    utls::sassert(distance(std::min(this->first_date_, o.first_date_),
                           std::max(this->last_date_, o.last_date_)) < BITSIZE,
                  "Covering more days than possible with only {} bits.",
                  BITSIZE);

    if (this->first_date_ < o.first_date_) {
      o.bitfield_ >> distance(this->first_date_, o.first_date_);
    } else {
      this->bitfield_ >> distance(o.first_date_, this->first_date_);
    }

    this->bitfield_ |= o.bitfield_;

    this->last_date_ = std::max(this->last_date_, o.last_date_);

    return *this;
  }

  date::year_month_day first_date_{};
  date::year_month_day last_date_{};

  cista::bitset<BITSIZE> bitfield_{};
};

}  // namespace soro::tt