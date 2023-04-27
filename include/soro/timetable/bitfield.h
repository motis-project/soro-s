#pragma once

#include "cista/containers/bitset.h"

#include "soro/base/soro_types.h"
#include "soro/base/time.h"

namespace soro::tt {

// covers the range [first_date_, last_date_]
struct bitfield {
  static constexpr const std::size_t BITSIZE = 512;
  using bitset = cista::bitset<BITSIZE>;
  using anchor_time = date::sys_days;

  static constexpr auto INVALID_ANCHOR = anchor_time::max();

  static constexpr bool valid(anchor_time const t) {
    return t != anchor_time::max();
  }

  struct iterator {
    using iterator_category = typename std::input_iterator_tag;
    using value_type = anchor_time;
    using difference_type = value_type;
    using pointer = value_type*;
    using reference = value_type;

    using idx_t = uint16_t;

    iterator(bitfield const* bitfield, idx_t const idx);

    iterator& operator++();
    value_type operator*() const;

    bool operator==(iterator const& other) const = default;
    bool operator!=(iterator const& other) const = default;

    bitfield const* bitfield_{nullptr};
    idx_t idx_{0};
  };

  iterator begin() const;
  iterator end() const;

  iterator from(absolute_time const t) const;
  iterator to(absolute_time const t) const;

  std::vector<anchor_time> get_set_days() const;

  bool ok() const noexcept;

  bool at(anchor_time const ymd) const;

  bool operator[](anchor_time const ymd) const;

  void set(anchor_time const ymd, bool const new_value);

  bool equivalent(bitfield const& o) const noexcept;

  bool operator==(bitfield const& o) const noexcept = default;

  bitfield operator|=(bitfield const& o) noexcept;
  friend bitfield operator|(bitfield const& lhs, bitfield const& rhs) noexcept;

  soro::size_t count() const noexcept;

  absolute_time first_date() const noexcept;
  absolute_time last_date() const noexcept;

  anchor_time first_date_{anchor_time::max()};
  anchor_time last_date_{anchor_time::max()};
  bitset bitset_{};
};

bitfield make_bitfield(bitfield::anchor_time const first_date,
                       bitfield::anchor_time const last_date,
                       const char* const bitmask);

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