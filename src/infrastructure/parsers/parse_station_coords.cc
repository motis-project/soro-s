#include "soro/infrastructure/parsers/parse_station_coords.h"

#include "utl/logging.h"
#include "utl/parser/buf_reader.h"
#include "utl/parser/csv_range.h"
#include "utl/parser/line_range.h"

#include "utl/pipes.h"

#include "soro/utls/file/loaded_file.h"
#include "soro/utls/parse_fp.h"

namespace soro::infra {

using namespace utl;
using namespace soro::utls;

soro::vector<gps> parse_station_coords(
    std::filesystem::path const& gps_path,
    soro::map<soro::string, station::ptr> const& ds100_to_station) {
  if (!std::filesystem::exists(gps_path)) {
    uLOG(warn) << "GPS coordinate path '" << gps_path << "' does not exist.";
    return {};
  }

  soro::vector<gps> station_coords(ds100_to_station.size());

  auto const gps_content = utls::read_file_to_string(gps_path);

  struct row {
    utl::csv_col<utl::cstr, UTL_NAME("ds100")> ds100_;
    utl::csv_col<utl::cstr, UTL_NAME("lat")> lat_;
    utl::csv_col<utl::cstr, UTL_NAME("lon")> lon_;
  };

  utl::line_range{utl::buf_reader{gps_content}} | utl::csv<row>() |
      utl::for_each([&](auto&& row) {
        auto s_it = ds100_to_station.find(row.ds100_.val().to_str());
        if (s_it == std::end(ds100_to_station)) {
          return;
        }
        auto lon_str = row.lon_.val().to_str();
        auto lat_str = row.lat_.val().to_str();

        gps::precision lon{gps::INVALID};
        gps::precision lat{gps::INVALID};

        if (!lon_str.empty()) {
          lon = parse_fp<gps::precision, replace_comma::ON>(lon_str);
        }

        if (!lat_str.empty()) {
          lat = parse_fp<gps::precision, replace_comma::ON>(lat_str);
        }

        station_coords[s_it->second->id_] = {.lon_ = lon, .lat_ = lat};
      });

  return station_coords;
}

}  // namespace soro::infra