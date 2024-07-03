#include "soro/server/modules/infrastructure/parse_station_positions.h"

#include <filesystem>

#include "utl/logging.h"
#include "utl/parser/buf_reader.h"
#include "utl/parser/cstr.h"
#include "utl/parser/csv_range.h"
#include "utl/parser/line_range.h"
#include "utl/pipes/for_each.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/coordinates/gps.h"
#include "soro/utls/file/loaded_file.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/sassert.h"

#include "soro/infrastructure/station/station.h"

namespace soro::server {

using namespace utl;
using namespace soro::utls;
using namespace soro::infra;

soro::map<station::ds100, utls::gps> parse_station_positions(
    std::filesystem::path const& station_positions_csv) {
  utl::scoped_timer const timer("parsing station positions");

  soro::map<station::ds100, utls::gps> station_positions;

  if (!std::filesystem::exists(station_positions_csv)) {
    uLOG(warn) << "GPS coordinate path '" << station_positions_csv
               << "' does not exist.";
    return station_positions;
  }

  auto const gps_content = utls::read_file_to_string(station_positions_csv);

  struct row {
    utl::csv_col<utl::cstr, UTL_NAME("ds100")> ds100_;
    utl::csv_col<utl::cstr, UTL_NAME("lat")> lat_;
    utl::csv_col<utl::cstr, UTL_NAME("lon")> lon_;
  };

  utl::line_range{utl::buf_reader{gps_content}} | utl::csv<row>() |
      utl::for_each([&](auto&& row) {
        auto lon_str = row.lon_.val().to_str();
        auto lat_str = row.lat_.val().to_str();

        gps result;

        if (!lon_str.empty()) {
          result.lon_ = parse_fp<gps::precision, replace_comma::ON>(lon_str);
        }

        if (!lat_str.empty()) {
          result.lat_ = parse_fp<gps::precision, replace_comma::ON>(lat_str);
        }

        utls::sassert(result.valid());

        station_positions[row.ds100_.val().to_str()] = result;
      });

  return station_positions;
}

}  // namespace soro::server