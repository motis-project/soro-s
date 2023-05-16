#include "soro/infrastructure/parsers/iss/parse_track_element.h"

#include "utl/logging.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/infrastructure/graph/graph_creation.h"
#include "soro/si/units.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/parse_helpers.h"

using namespace pugi;
using namespace soro::si;
using namespace soro::utls;

namespace soro::infra {

speed_limit::poa parse_speed_limit_poa(xml_node const& poa_node) {
  switch (str_hash(poa_node.child_value())) {
    case str_hash(SIGNAL): {
      return speed_limit::poa::HERE;
    }

    case str_hash(HERE): {
      return speed_limit::poa::LAST_SIGNAL;
    }

    default: {
      throw utl::fail("Unkown speed limit signifier {}",
                      poa_node.child_value());
    }
  }
}

speed_limit::effects parse_speed_limit_effects(xml_node const& effects_node) {
  switch (str_hash(effects_node.child_value())) {
    case str_hash(ALL): {
      return speed_limit::effects::ALL;
    }

    case str_hash(CONVENTIONAL): {
      return speed_limit::effects::CONVENTIONAL;
    }

    case str_hash(LZB): {
      return speed_limit::effects::LZB_ETCS;
    }

    default: {
      throw utl::fail("Unkown speed limit signifier {}",
                      effects_node.child_value());
    }
  }
}

speed_limit get_speed_limit(xml_node const& speed_limit_xml,
                            element_ptr element) {
  speed_limit spl;

  if (utls::equal(speed_limit_xml.name(), SPECIAL_SPEED_LIMIT_END_FALLING) ||
      utls::equal(speed_limit_xml.name(), SPECIAL_SPEED_LIMIT_END_RISING)) {
    spl.type_ = speed_limit::type::END_SPECIAL;
  }

  if (utls::equal(speed_limit_xml.name(), SPEED_LIMIT_RISING) ||
      utls::equal(speed_limit_xml.name(), SPEED_LIMIT_FALLING) ||
      utls::equal(speed_limit_xml.name(), SPEED_LIMIT)) {
    spl.type_ = speed_limit::type::GENERAL_ALLOWED;
  }

  if (auto const length = speed_limit_xml.child("Laenge");
      static_cast<bool>(length)) {
    auto l = parse_fp<si::precision, replace_comma::ON>(
        speed_limit_xml.child_value("Laenge"));
    if (strcmp(length.attribute("Einheit").value(), "km") == 0) {
      spl.length_ = si::from_km(l);
    } else {
      spl.length_ = si::from_m(l);
    }
  }

  if (auto const s = speed_limit_xml.child("Geschw"); static_cast<bool>(s)) {
    spl.limit_ = si::from_km_h(parse_fp<si::precision, replace_comma::ON>(
        speed_limit_xml.child_value("Geschw")));
  }

  if (si::valid(spl.limit_) && spl.limit_ == si::ZERO<si::speed>) {
    uLOG(utl::warn) << "Got speed limit with 0 km/h limit.";
  }

  spl.calculated_ = static_cast<bool>(speed_limit_xml.child("Berechnet"));
  spl.poa_ = parse_speed_limit_poa(speed_limit_xml.child("Wirkungsort"));
  spl.effects_ =
      parse_speed_limit_effects(speed_limit_xml.child("Zugbeeinflussung"));

  spl.element_ = element;

  if (element->is_track_element()) {
    spl.node_ = element->as<track_element>().get_node();
  }

  return spl;
}

slope get_slope(xml_node const& track_element_node) {
  auto const rising_slope = per_mille{parse_fp<precision, replace_comma::ON>(
      track_element_node.child_value("Steigend"))};
  auto const falling_slope = per_mille{parse_fp<precision, replace_comma::ON>(
      track_element_node.child_value("Fallend"))};

  return slope{.rising_ = rising_slope, .falling_ = falling_slope};
}

main_signal get_main_signal(xml_node const main_signal_node) {
  auto const name = main_signal_node.child_value("Name");
  return main_signal{.name_ = name};
}

halt get_halt(pugi::xml_node const& node) {
  halt hd;

  hd.name_ = node.child("Name").child_value();
  hd.identifier_operational_ = node.child("BezeichnerBetrieb").child_value();
  hd.identifier_extern_ = node.child("BezeichnerExtern").child_value();

  bool const name_contains_rz = std::strstr(node.name(), "Rz") != nullptr;
  bool const name_contains_left = std::strstr(node.name(), "Links") != nullptr;
  bool const name_contains_right =
      std::strstr(node.name(), "Rechts") != nullptr;

  hd.is_passenger_ = name_contains_rz;
  hd.is_left_ = name_contains_left;
  hd.is_right_ = name_contains_right;

  return hd;
}

eotd get_eotd(pugi::xml_node const& node) {
  bool const signal_eotd = utls::equal(node.name(), SIGNAL_EOTD_FALLING) ||
                           utls::equal(node.name(), SIGNAL_EOTD_RISING);

  return eotd{.signal_ = signal_eotd};
}

void add_element_data(pugi::xml_node const& xml_node, element* element,
                      graph& network) {
  auto const element_id = element->id();

  sassert(network.element_data_.size() > element_id,
          "Element data vector is not sufficiently sized!");

  if (element->is(type::HALT)) {
    network.element_data_[element_id].emplace<halt>(get_halt(xml_node));
  } else if (element->is(type::SLOPE)) {
    network.element_data_[element_id].emplace<slope>(get_slope(xml_node));
  } else if (element->is(type::EOTD)) {
    network.element_data_[element_id].emplace<eotd>(get_eotd(xml_node));
  } else if (element->is(type::SPEED_LIMIT)) {
    auto const spl = get_speed_limit(xml_node, element);

    utls::sassert(spl.node_ != nullptr,
                  "Could not find speed limit node for track element!");

    network.element_data_[element_id].emplace<speed_limit>(spl);
  } else if (element->is(type::MAIN_SIGNAL)) {
    network.element_data_[element_id].emplace<main_signal>(
        get_main_signal(xml_node));
  }
}

element* parse_track_element(xml_node const& track_node, type const type,
                             bool const rising, line::id const line,
                             graph& network, station& station,
                             construction_materials& mats) {
  auto element = create_element(network, station, mats, type,
                                parse_rp_node_id(track_node), rising);

  set_km_point_and_line(
      *element, track_node.name(),
      parse_kilometrage(track_node.child_value(KILOMETER_POINT)), line);

  add_element_data(track_node, element, network);

  return element;
}

}  // namespace soro::infra