#pragma once

#include "soro/base/soro_types.h"

#include "soro/rolling_stock/train_type.h"

namespace soro::rs {

struct train_category {
  struct key {
    CISTA_COMPARABLE()

    using main = soro::strong<int32_t, struct _train_category_main>;
    using sub = soro::strong<int32_t, struct _train_category_sub>;

    main main_;
    // TODO(julian) sub and name might be redundant
    sub sub_;
    soro::string name_;
  };

  bool is_ice() const;

  key key_;
  soro::string main_description_;
  soro::string sub_description_;
  rs::train_type type_;
};

}  // namespace soro::rs
