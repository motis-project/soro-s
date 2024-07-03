#include "soro/rolling_stock/parsers/iss/parse_rolling_stock.h"

#include "utl/timer.h"

#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"

#include "soro/rolling_stock/parsers/iss/parse_train_categories.h"
#include "soro/rolling_stock/parsers/iss/parse_train_classes.h"
#include "soro/rolling_stock/parsers/iss/parse_train_series.h"
#include "soro/rolling_stock/rolling_stock.h"

namespace soro::rs {

using namespace soro::infra;

rs::rolling_stock parse_rolling_stock(iss_files const& iss_files,
                                      dictionaries const& dicts) {
  utl::scoped_timer const rolling_stock_timer("parsing rolling stock");

  rs::rolling_stock rs;

  rs.train_series_ = parse_train_series(iss_files, dicts);
  rs.train_classes_ = parse_train_classes(iss_files);
  rs.train_categories_ = parse_train_categories(iss_files);

  return rs;
}

}  // namespace soro::rs
