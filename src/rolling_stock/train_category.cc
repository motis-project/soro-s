#include "soro/rolling_stock/train_category.h"

namespace soro::rs {

bool train_category::is_ice() const { return this->key_.name_ == "ICE"; }

}  // namespace soro::rs
