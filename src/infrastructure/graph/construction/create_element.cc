#include "soro/infrastructure/graph/construction/create_element.h"

#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"

#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/station/station.h"

namespace soro::infra {

using namespace utl;
using namespace soro::si;
using namespace soro::utls;

element* create_element(graph& network, station& station,
                        construction_materials& mats, type const type,
                        rail_plan_node_id const rp_id) {
  auto const opt = soro::optional<rail_plan_node_id>{rp_id};
  if (is_cross(type)) {
    return create_element_t<cross>(network, station, mats, type, opt);
  } else if (is_end_element(type)) {
    return create_element_t<end_element>(network, station, mats, type, opt);
  } else if (is_simple_element(type)) {
    return create_element_t<simple_element>(network, station, mats, type, opt);
  } else if (is_simple_switch(type)) {
    return create_element_t<simple_switch>(network, station, mats, type, opt);
  } else if (is_track_element(type)) {
    return create_element_t<track_element>(network, station, mats, type, opt);
  }

  throw utl::fail("Could not dispatch create element for type: {}", type);
}

element* get_or_create_element(graph& network, station& station,
                               construction_materials& mats, type const type,
                               rail_plan_node_id const rp_id) {
  if (mats.to_element_id_.size() <= rp_id ||
      mats.to_element_id_.first(rp_id) == element::invalid()) {
    return create_element(network, station, mats, type, rp_id);
  } else {
    utls::sassert(mats.to_element_id_.second(rp_id) == element::invalid(),
                  "second id should be invalid as we only get or create "
                  "section elements");
    return network.element_store_[as_val(mats.to_element_id_.first(rp_id))]
        .get();
  }
}

}  // namespace soro::infra
