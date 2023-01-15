#include "soro/infrastructure/parsers/iss/parse_helpers.h"

#include "soro/utls/parse_fp.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using namespace soro::utls;

kilometrage parse_kilometrage(pugi::xml_node const& node) {
  std::string_view const kmp(node.child_value(KILOMETER_POINT));

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
  return std::stoull(node.child_value(ID));
}

bool has_rising_name(pugi::xml_node const& node) {
  auto const& name = node.name();
  return name[strlen(name) - 1] == 'S';
}

}  // namespace soro::infra