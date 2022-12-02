#pragma once

#include "cista/containers/bitset.h"

#include "soro/base/time.h"

namespace soro::tt {

// covers the range [first_date_, last_date_]
struct bitfield {
  static constexpr const std::size_t BITSIZE = 512;
  using bitset = cista::bitset<BITSIZE>;

  std::vector<date::year_month_day> get_set_days() const;

  bool ok() const noexcept;

  bool at(anchor_time const ymd) const;

  bool operator[](anchor_time const ymd) const;

  void set(date::year_month_day const ymd, bool const new_value);

  bool equivalent(bitfield const& o) const noexcept;

  bool operator==(bitfield const& o) const noexcept = default;

  bitfield operator|=(bitfield const& o) noexcept;
  friend bitfield operator|(bitfield const& lhs, bitfield const& rhs) noexcept;

  // with BITSIZE the bitfield can cover a range of [first_date_, end())
  anchor_time end() const noexcept;

  std::size_t count() const noexcept;

  anchor_time first_date_{INVALID<anchor_time>};
  anchor_time last_date_{INVALID<anchor_time>};
  bitset bitset_{};
};

bitfield make_bitfield(date::year_month_day first_date,
                       date::year_month_day last_date,
                       const char* const bitmask);

std::size_t distance(date::year_month_day from, date::year_month_day to);

}  // namespace soro::tt