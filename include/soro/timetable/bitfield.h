#pragma once

#include "cista/containers/bitset.h"
#include "date/date.h"

namespace soro::tt {

struct bitfield {
  static constexpr const std::size_t BITSIZE = 512;

  bitfield() = default;
  bitfield(date::year_month_day start, date::year_month_day end,
           std::string bit_string);

  std::vector<date::year_month_day> get_set_days() const;

  bool ok() const noexcept;

  bool at(date::year_month_day const ymd) const;

  bool operator[](date::year_month_day const ymd) const noexcept;

  void set(date::year_month_day const ymd, bool const new_value);

  bool equivalent(bitfield const& o) const noexcept;

  bool operator==(bitfield const& o) const noexcept = default;

  bitfield operator|=(bitfield const& o) noexcept;
  friend bitfield operator|(bitfield const& lhs, bitfield const& rhs) noexcept;

  date::year_month_day first_date_{};
  date::year_month_day last_date_{};

  cista::bitset<BITSIZE> bitset_{};
};

}  // namespace soro::tt