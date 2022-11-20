#pragma once

#include "cista/containers/bitset.h"
#include "date/date.h"

namespace soro::tt {

// covers the range [first_date_, last_date_]
struct bitfield {
  static constexpr const std::size_t BITSIZE = 512;
  using bitset = cista::bitset<BITSIZE>;

  std::vector<date::year_month_day> get_set_days() const;

  bool ok() const noexcept;

  bool at(date::year_month_day const ymd) const;

  bool operator[](date::year_month_day const ymd) const;

  void set(date::year_month_day const ymd, bool const new_value);

  bool equivalent(bitfield const& o) const noexcept;

  bool operator==(bitfield const& o) const noexcept = default;

  bitfield operator|=(bitfield const& o) noexcept;
  friend bitfield operator|(bitfield const& lhs, bitfield const& rhs) noexcept;

  // with BITSIZE the bitfield can cover a range of [first_date_, end())
  date::year_month_day end() const noexcept;

  std::size_t count() const noexcept;

  date::year_month_day first_date_{};
  date::year_month_day last_date_{};
  bitset bitset_{};
};

bitfield make_bitfield(date::year_month_day first_date,
                       date::year_month_day last_date,
                       const char* const bitmask);

std::size_t distance(date::year_month_day from, date::year_month_day to);

}  // namespace soro::tt