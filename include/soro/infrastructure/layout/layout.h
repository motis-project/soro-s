#pragma once

#include "pugixml.hpp"

#include "soro/utls/string.h"

#include "soro/infrastructure/infrastructure_t.h"
#include "soro/infrastructure/parsers/iss/iss_types.h"

namespace soro::layout {

struct coordinates {
  using precision = double;

  static constexpr precision INVALID = std::numeric_limits<precision>::max();

  precision x_{INVALID};
  precision y_{INVALID};
};

// size = number of elements
// every element needs a placement
using layout = std::vector<coordinates>;

coordinates parse_coordinates(pugi::xml_node const& coord_child);

layout get_layout(std::vector<utls::loaded_file> const& rail_plan_files,
                  soro::vector<infra::station::ptr> const& stations,
                  soro::vector<infra::section> const& sections,
                  soro::map<infra::rail_plan_node_id, infra::element_id> const&
                      rp_id_to_element_id,
                  std::size_t const element_count);

}  // namespace soro::layout
