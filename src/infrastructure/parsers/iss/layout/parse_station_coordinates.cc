#include "soro/infrastructure/parsers/iss/layout/parse_station_coordinates.h"

#include <iterator>

#include "pugixml.hpp"

#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"

#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/layout.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/layout/parse_coordinates.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

soro::vector_map<station::id, coordinates> parse_station_coordinates(
    iss_files const& iss_files, infrastructure_t const& infra) {
  utl::scoped_timer const regulartory_timer("parsing station coordinates");

  utls::expect(!infra.ds100_to_station_.empty(), "need ds100 to station");

  soro::vector_map<station::id, coordinates> result(infra.stations_.size());

  for (auto const& reg_file_xml : iss_files.regulatory_station_files_) {
    for (auto const& regulatory_station :
         reg_file_xml.child(XML_ISS_DATA).child(REGULATORIES).child(STATIONS)) {
      soro::string const ds100 = regulatory_station.child_value(DS100);

      if (auto pos_xml = regulatory_station.child(POSITION); pos_xml) {
        if (auto it = infra.ds100_to_station_.find(ds100);
            it != std::end(infra.ds100_to_station_)) {
          result[it->second->id_] = parse_coordinates(pos_xml);
        }
      }
    }
  }

  // TODO(julian) the small example data set has no station coordinates
  // utls::ensure(utls::all_of(result, [](auto&& c) { return c.valid(); }));

  return result;
}

}  // namespace soro::infra