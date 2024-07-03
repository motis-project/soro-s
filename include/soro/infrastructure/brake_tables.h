#pragma once

#include "soro/utls/container/static_vector.h"

#include "soro/rolling_stock/train_physics.h"

#include "soro/infrastructure/brake_path.h"

namespace soro::infra {

struct brake_table {
  struct line {
    using length = uint8_t;
    using index = soro::strong<uint8_t, struct _brake_table_line_idx>;

    static constexpr si::speed step_size = si::from_km_h(5.0);
    // 160 km/h in 5km/h steps
    static constexpr length max_line_length = (160 / 5) + 1;

    using entries =
        utls::static_vector<rs::brake_weight_percentage, max_line_length>;

    static index get_idx(si::speed const speed);
    index get_idx(rs::brake_weight_percentage const percentage) const;

    static si::speed get_speed(index const idx);
    si::speed get_speed(rs::brake_weight_percentage const percentage) const;

    rs::brake_weight_percentage get_percentage(index const idx) const;
    rs::brake_weight_percentage get_percentage(si::speed const speed) const;

    rs::brake_weight_percentage const& operator[](index const idx) const;
    rs::brake_weight_percentage& operator[](index const idx);

    entries::const_iterator begin() const;
    entries::const_iterator end() const;

    length size() const;
    bool empty() const;

    bool has_entry(index const idx) const;

    bool operator<(line const& other) const;

    si::slope slope_;
    entries entries_;
  };

  // slopes are given from 0 to 68 per mille gradient
  static constexpr uint8_t line_count = 69;
  using lines = utls::static_vector<line, line_count>;

  bool empty() const;

  si::speed get_max_speed(
      si::slope const slope,
      rs::brake_weight_percentage const train_percentage) const;

  si::speed interpolate(
      lines::const_iterator const it, si::slope const slope,
      rs::brake_weight_percentage const train_percentage) const;

  lines lines_{lines::max_size};
};

struct brake_tables {
  using paths = soro::vector_map<brake_path, brake_table>;
  using types = soro::vector_map<rs::brake_type, paths>;

  si::speed get_max_speed(rs::brake_type, brake_path, si::slope,
                          rs::brake_weight_percentage) const;

  types types_;
};

}  // namespace soro::infra
