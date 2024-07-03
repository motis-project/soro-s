#include "soro/infrastructure/parsers/iss/layout/parse_coordinates.h"

#include "soro/utls/parse_fp.h"

#include "soro/infrastructure/layout.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using namespace utls;

coordinates parse_coordinates(pugi::xml_node const& coord_child) {
  return {.x_ = parse_fp<coordinates::precision, replace_comma::ON>(
              coord_child.child_value(X)),
          .y_ = parse_fp<coordinates::precision, replace_comma::ON>(
              coord_child.child_value(Y))};
}

}  // namespace soro::infra