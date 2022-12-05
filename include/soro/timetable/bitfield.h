#pragma once

#include "cista/containers/bitset.h"

#include "soro/base/time.h"

namespace soro::tt {

// covers the range [first_date_, last_date_]
struct bitfield {
  static constexpr const std::size_t BITSIZE = 512;
  using bitset = cista::bitset<BITSIZE>;
  using anchor_time = date::sys_days;

  static constexpr bool valid(anchor_time const t) {
    return t != anchor_time ::max();
  }

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

  anchor_time first_date_{anchor_time::max()};
  anchor_time last_date_{anchor_time::max()};
  bitset bitset_{};
};

bitfield make_bitfield(date::year_month_day first_date,
                       date::year_month_day last_date,
                       const char* const bitmask);

std::size_t distance(date::year_month_day from, date::year_month_day to);

}  // namespace soro::tt

template <>
struct fmt::formatter<soro::tt::bitfield::anchor_time> {
  constexpr auto parse(format_parse_context& ctx)  // NOLINT
      -> decltype(ctx.begin()) {
    if (ctx.begin() != ctx.end() && *ctx.begin() != '}') {
      throw format_error("invalid format for anchor time");
    }

    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(soro::tt::bitfield::anchor_time const at,
              FormatContext& ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(ctx.out(), "{}", date::year_month_day{at});
  }
};