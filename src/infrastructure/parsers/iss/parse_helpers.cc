#include "soro/infrastructure/parsers/iss/parse_helpers.h"

#include <string>
#include <string_view>

#include "utl/verify.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/si/units.h"

#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using namespace soro::utls;

kilometrage parse_kilometrage(std::string_view const kmp) {
  if (auto const pos = kmp.find('+'); pos != std::string::npos) {
    auto const base_km =
        parse_fp<kilometrage::precision, utls::replace_comma::ON>(
            kmp.substr(0, pos));
    auto const extra_km =
        parse_fp<kilometrage::precision, utls::replace_comma::ON>(
            kmp.substr(pos + 1, kmp.size()));

    return si::from_km(base_km) + si::from_km(extra_km);
  } else {
    return si::from_km(
        parse_fp<kilometrage::precision, utls::replace_comma::ON>(kmp));
  }
}

rail_plan_node_id parse_rp_node_id(pugi::xml_node const& node) {
  return utls::parse_int<rail_plan_node_id>(node.child_value(ID));
}

rail_plan_node_id parse_rp_node_id_attribute(pugi::xml_node const& node) {
  return utls::parse_int<rail_plan_node_id>(node.attribute(ID).value());
}

mileage_dir get_track_element_direction(pugi::xml_node const& node) {
  utls::expect(is_track_element(get_type(node.name())),
               "element {} not an track element", node.name());

  auto const last_char_is = [](auto&& s, char const c) {
    return s[strlen(s) - 1] == c;
  };

  auto const name = node.name();
  auto const type = get_type(name);

  utls::sassert(is_undirected_track_element(type) || last_char_is(name, 'F') ||
                    last_char_is(name, 'S'),
                "cannot determine mileage direction for {}", name);

  return is_undirected_track_element(type) ? mileage_dir::undirected
         : last_char_is(name, 'S')         ? mileage_dir::rising
                                           : mileage_dir::falling;
}

si::speed parse_speed(pugi::xml_node const& speed_xml) {
  auto const unit = speed_xml.attribute(UNIT).value();
  auto const v = parse_fp<si::speed::precision>(speed_xml.child_value());

  if (utls::equal(unit, "km/h")) {
    return si::from_km_h(v);
  } else if (utls::equal(unit, "m/s")) {
    return si::from_m_s(v);
  } else {
    throw utl::fail("could not determine unit for speed, got {}", unit);
  }
}

si::length parse_length(pugi::xml_node const& length_xml) {
  auto const unit = length_xml.attribute(UNIT).value();
  auto const v = parse_fp<si::length::precision, replace_comma::ON>(
      length_xml.child_value());

  utls::sassert(v >= 0.0, "got negative length {}", v);

  if (utls::equal(unit, "km")) {
    return si::from_km(v);
  } else if (utls::equal(unit, "m")) {
    return si::from_m(v);
  } else {
    throw utl::fail("could not determine unit for length, got {}", unit);
  }
}

}  // namespace soro::infra