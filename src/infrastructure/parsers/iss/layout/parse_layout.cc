#include "soro/infrastructure/parsers/iss/layout/parse_layout.h"

#include "utl/timer.h"

#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/layout.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/layout/get_node_coordinates.h"
#include "soro/infrastructure/parsers/iss/layout/parse_element_coordinates.h"
#include "soro/infrastructure/parsers/iss/layout/parse_station_coordinates.h"

namespace soro::infra {

layout parse_layout(iss_files const& iss_files, infrastructure_t const& infra,
                    construction_materials const& mats) {
  utl::scoped_timer const timer("parsing layout");

  layout result;

  result.station_coordinates_ = parse_station_coordinates(iss_files, infra);
  result.element_coordinates_ =
      parse_element_coordinates(iss_files, infra, mats);
  result.node_coordinates_ =
      get_node_coordinates(result.element_coordinates_, infra);

  return result;
};

}  // namespace soro::infra