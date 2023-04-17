#include "soro/rolling_stock/rolling_stock_error.h"

namespace {

struct rs_error_category : std::error_category {
  const char* name() const noexcept override;
  std::string message(int ev) const override;
};

const char* rs_error_category::name() const noexcept { return "rs"; }

std::string rs_error_category::message(int ev) const {
  switch (static_cast<soro::error::rolling_stock>(ev)) {
    case soro::error::rolling_stock::TRACTION_VEHICLE_NOT_FOUND:;
      return "could not find rolling stock vehicle";
  }

  return "not reachable";
}

const rs_error_category kss_error_category_singleton{};

}  // namespace

namespace soro::error {

std::error_code make_error_code(rolling_stock const e) {
  return {static_cast<int>(e), kss_error_category_singleton};
}

}  // namespace soro::error
