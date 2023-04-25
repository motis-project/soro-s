#include "soro/infrastructure/parsers/iss/parse_helpers.h"

#include "soro/utls/parse_fp.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::infra {

using namespace soro::utls;

kilometrage parse_kilometrage(std::string_view const kmp) {
  auto const sign_pos = kmp.find_first_of("+-");

  if (sign_pos == std::string::npos || sign_pos == 0) {
    return si::from_km(parse_fp<kilometrage::precision, utls::replace_comma::ON>(kmp.substr(0, kmp.size())));
  }

  auto km =
      parse_fp<kilometrage::precision, utls::replace_comma::ON>(kmp.substr(0, sign_pos));

  if (kmp[sign_pos] == '+') {
    km += parse_fp<kilometrage::precision, utls::replace_comma::ON>(kmp.substr(sign_pos + 1, kmp.size()));
  } else {
    km -= parse_fp<kilometrage::precision, utls::replace_comma::ON>(kmp.substr(sign_pos + 1, kmp.size()));
  }

  return si::from_km(km);
}

rail_plan_node_id parse_rp_node_id(pugi::xml_node const& node) {
  return std::stoull(node.child_value(ID));
}

bool has_rising_name(pugi::xml_node const& node) {
  auto const& name = node.name();
  return name[strlen(name) - 1] == 'S';
}

}  // namespace soro::infra