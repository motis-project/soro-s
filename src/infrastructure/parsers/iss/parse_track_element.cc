#include "soro/infrastructure/parsers/iss/parse_track_element.h"

#include <cstring>

#include "cista/containers/optional.h"

#include "utl/logging.h"
#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/rolling_stock/parsers/iss/parse_train_type.h"
#include "soro/rolling_stock/train_physics.h"
#include "soro/rolling_stock/train_series.h"
#include "soro/rolling_stock/train_type.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/graph/construction/create_element.h"
#include "soro/infrastructure/graph/construction/set_km.h"
#include "soro/infrastructure/graph/construction/set_line.h"
#include "soro/infrastructure/graph/element.h"
#include "soro/infrastructure/graph/element_data.h"
#include "soro/infrastructure/graph/graph.h"
#include "soro/infrastructure/graph/type.h"
#include "soro/infrastructure/kilometrage.h"
#include "soro/infrastructure/line.h"
#include "soro/infrastructure/parsers/iss/construction_materials.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"
#include "soro/infrastructure/parsers/iss/parse_helpers.h"
#include "soro/infrastructure/station/station.h"

#include "soro/si/units.h"

using namespace pugi;
using namespace soro::si;
using namespace soro::utls;

namespace soro::infra {

speed_limit::poa parse_speed_limit_poa(xml_node const& poa_node) {
  switch (str_hash(poa_node.child_value())) {
    case str_hash(HERE): {
      return speed_limit::poa::here;
    }

    case str_hash(SIGNAL): {
      return speed_limit::poa::last_signal;
    }

    default: {
      throw utl::fail("unkown speed limit point of activation {}",
                      poa_node.child_value());
    }
  }
}

speed_limit::affects parse_speed_limit_affects(xml_node const& affects_node) {
  switch (str_hash(affects_node.child_value())) {
    case str_hash(ALL): {
      return speed_limit::affects::all;
    }

    case str_hash(CONVENTIONAL): {
      return speed_limit::affects::conventional;
    }

    case str_hash(LZB): {
      return speed_limit::affects::ctc;
    }

    default: {
      throw utl::fail("Unkown speed limit signifier {}",
                      affects_node.child_value());
    }
  }
}

soro::vector<rs::train_series::key> parse_train_series(
    xml_node const& specials_xml) {
  soro::vector<rs::train_series::key> result;

  for (auto const& train_series_xml : specials_xml.children("TFZBaureihe")) {
    rs::train_series::key tsk;

    auto const number_val = train_series_xml.child_value();
    auto const company_val = train_series_xml.attribute("Nr").value();

    tsk.number_ = rs::train_series::key::number{number_val};
    tsk.company_ = rs::train_series::key::company{company_val};

    result.push_back(tsk);
  }

  return result;
}

soro::vector<rs::tilt_technology> parse_tilt_technologies(
    xml_node const& specials_xml, dictionaries const& dicts) {
  soro::vector<rs::tilt_technology> result;

  for (auto const& tilt_xml : specials_xml.children("Neigungstechnik")) {
    auto const tilt_key = parse_dictionary_key<rs::tilt_technology_key>(
        tilt_xml.attribute("Schluessel").value());
    result.emplace_back(dicts.tilt_technology_.get_id(tilt_key));
  }

  return result;
}

soro::vector<rs::train_series::type> parse_train_series_types(
    xml_node const& specials_xml, dictionaries const& dicts) {
  soro::vector<rs::train_series::type> result;

  for (auto const& series_type_xml : specials_xml.children("TFZArt")) {
    auto const series_type_key =
        parse_dictionary_key<rs::train_series::type_key>(
            series_type_xml.attribute("Schluessel").value());
    result.emplace_back(dicts.train_series_type_.get_id(series_type_key));
  }

  return result;
}

soro::vector<rs::transportation_specialty> parse_transportation_specialties(
    xml_node const& specials_xml, dictionaries const& dicts) {
  soro::vector<rs::transportation_specialty> result;

  for (auto const& specialty_xml :
       specials_xml.children("Beschreibungsvorlage")) {
    auto const key = parse_dictionary_key<rs::transportation_specialty_key>(
        specialty_xml.attribute("Schluessel").value());

    result.emplace_back(dicts.transportation_specialty_.get_id(key));
  }

  return result;
}

speed_limit::type parse_speed_limit_type(xml_node const& speed_limit_xml) {
  speed_limit::type result{speed_limit::type::invalid};

  auto const& name = speed_limit_xml.name();

  if (utls::equal(name, SPECIAL_SPEED_LIMIT_END_FALLING) ||
      utls::equal(name, SPECIAL_SPEED_LIMIT_END_RISING)) {
    result = speed_limit::type::end_special;
  } else if (utls::equal(name, SPEED_LIMIT_RISING) ||
             utls::equal(name, SPEED_LIMIT_FALLING) ||
             utls::equal(name, SPEED_LIMIT)) {
    result = speed_limit::type::general;
  } else if (utls::equal(name, SPEED_LIMIT_DIVERGENT_FALLING) ||
             utls::equal(name, SPEED_LIMIT_DIVERGENT_RISING)) {
    result = speed_limit::type::divergent;
  } else {
    uLOG(utl::warn) << "could not determine speed limit type from "
                    << speed_limit_xml.name();
  }

  // a general speed limit with specialties is a special speed limit
  if (result == speed_limit::type::general &&
      static_cast<bool>(speed_limit_xml.child("Besonderheiten"))) {
    result = speed_limit::type::special;
  }

  return result;
}

si::length parse_speed_limit_length(xml_node const& speed_limit_xml) {
  auto const length_xml = speed_limit_xml.child("Laenge");
  return static_cast<bool>(length_xml) ? parse_length(length_xml)
                                       : si::length::infinity();
}

si::speed parse_speed_limit_velocity(xml_node const& speed_limit_xml) {
  auto const speed_xml = speed_limit_xml.child("Geschw");
  return static_cast<bool>(speed_xml) ? parse_speed(speed_xml)
                                      : si::speed::invalid();
}

cista::optional<rs::train_type> try_parsing_train_type(
    xml_node const& specials_xml) {
  auto const& train_type_xml = specials_xml.child("ZugartFV");

  if (!static_cast<bool>(train_type_xml)) return {};

  return rs::parse_train_type(train_type_xml);
}

speed_limit get_speed_limit(xml_node const& speed_limit_xml,
                            dictionaries const& dicts,
                            speed_limit::source const source) {
  speed_limit spl;

  spl.type_ = parse_speed_limit_type(speed_limit_xml);
  spl.length_ = parse_speed_limit_length(speed_limit_xml);
  spl.limit_ = parse_speed_limit_velocity(speed_limit_xml);
  spl.calculated_ = static_cast<bool>(speed_limit_xml.child("Berechnet"));
  spl.poa_ = parse_speed_limit_poa(speed_limit_xml.child("Wirkungsort"));
  spl.source_ = source;
  spl.affects_ =
      parse_speed_limit_affects(speed_limit_xml.child("Zugbeeinflussung"));

  utls::ensure(!(spl.from_last_signal() && spl.length_ < si::length::max()),
               "invalid combination");

  auto const& specials_xml = speed_limit_xml.child("Besonderheiten");
  spl.tilt_technologies_ = parse_tilt_technologies(specials_xml, dicts);
  spl.train_series_types_ = parse_train_series_types(specials_xml, dicts);
  spl.train_series_ = parse_train_series(specials_xml);
  spl.transportation_specialties_ =
      parse_transportation_specialties(specials_xml, dicts);
  spl.train_type_ = try_parsing_train_type(specials_xml);

  utls::ensures([&] {
    auto const has_specialty =
        spl.train_type_.has_value() || !spl.train_series_.empty() ||
        !spl.train_series_types_.empty() || !spl.tilt_technologies_.empty() ||
        !spl.transportation_specialties_.empty();

    utls::ensure(!spl.is_special() || has_specialty,
                 "special speed limits requires specialty");
    utls::ensure(!spl.is_general() || !has_specialty,
                 "general speed limits cannot have specialties");
  });

  utls::ensure(spl.ends_special() || spl.limit_.is_valid());
  utls::ensure(spl.ends_special() || !spl.limit_.is_infinity());

  utls::ensure(spl.ends_special() || spl.length_.is_valid());

  return spl;
}

slope get_slope(xml_node const& track_element_node, mileage_dir const dir) {
  auto const child_name = is_rising(dir) ? "Steigend" : "Fallend";

  auto const gradient =
      from_per_mille_gradient(parse_fp<si::slope::precision, replace_comma::ON>(
          track_element_node.child_value(child_name)));

  return gradient;
}

/* TODO(julian) unused for now, create appropriate types for voltage types etc.
rs::electric_configuration get_electrification(
    xml_node const track_element_node, mileage_dir const dir,
    dictionaries const& dicts) {
  auto const child_name =
      is_rising(dir) ? "SpannungsartSteigend" : "SpannungsartFallend";

  auto const child_node = track_element_node.child(child_name);

  // no electrification in this direction, return empty optional
  if (!static_cast<bool>(child_node)) return {};

  auto const current_key = parse_dictionary_key<rs::electric_configuration_key>(
      track_element_node.child(child_name).attribute("Schluessel").value());

  return rs::electric_configuration{
      dicts.electric_current_.get_id(current_key)};
}

rs::electric_configuration get_electrification_repeater(
    xml_node const& track_element_node, dictionaries const& dicts) {
  auto const current_xml = track_element_node.child("Spannungsart");

  auto const current_key = parse_dictionary_key<rs::electric_configuration_key>(
      current_xml.attribute("Schluessel").value());

  return rs::electric_configuration{
      dicts.electric_current_.get_id(current_key)};
}
 */

approach_signal get_approach_signal(xml_node const approach_signal_node) {
  auto const name = approach_signal_node.child_value("Name");
  return approach_signal{.name_ = name};
}

main_signal get_main_signal(xml_node const main_signal_node) {
  auto const name = main_signal_node.child_value("Name");

  auto const skip_xml = main_signal_node.child("AnzahlVSigBisRelevant");
  auto const skip_approach =
      static_cast<bool>(skip_xml)
          ? parse_int<soro::size_t>(
                main_signal_node.child_value("AnzahlVSigBisRelevant"))
          : 1;

  auto const type = main_signal_node.child_value("Signalart");

  return main_signal{
      .name_ = name, .type_ = type, .skip_approach_ = skip_approach};
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

brake_path get_brake_path(pugi::xml_node const& node,
                          dictionaries const& dicts) {
  auto const val = node.child("Wert").attribute("Schluessel").value();
  auto const key = parse_dictionary_key<brake_path_key>(val);
  return dicts.brake_path_.get_id(key);
}

lzb_start get_lzb_start(pugi::xml_node const& node) {
  lzb_start result;

  auto const& val = node.child_value("MassgebendeNeigung");
  result.avg_slope_ = from_per_mille_gradient(
      parse_fp<si::slope::precision, replace_comma::ON>(val));

  return result;
}

lzb_block_sign get_lzb_block_sign(pugi::xml_node const& node) {
  lzb_block_sign result;

  result.name_ = node.child_value("Name");

  if (static_cast<bool>(node.child("BZBezeichner"))) {
    result.control_name_ = node.child_value("BZBezeichner");
  }

  if (static_cast<bool>(node.child("Oberflaechenbezeichner"))) {
    result.ui_name_ = node.child_value("Oberflaechenbezeichner");
  }

  auto const time_val = node.child_value("FstrBildezeit");
  utls::sassert(
      utls::equal(node.child("FstrBildezeit").attribute(UNIT).value(), "s"));
  result.interlocking_time_ =
      si::from_s(utls::parse_fp<si::time::precision>(time_val));

  return result;
}

void add_element_data(pugi::xml_node const& xml_node, element* element,
                      graph& network, mileage_dir const dir,
                      dictionaries const& dicts) {
  auto const element_id = element->get_id();

  utls::sassert(network.element_data_.size() > as_val(element_id),
                "element data vector is not sufficiently sized!");

  if (element->is(type::HALT)) {
    network.element_data_[element_id].emplace<halt>(get_halt(xml_node));
  } else if (element->is(type::SLOPE)) {
    network.element_data_[element_id].emplace<slope>(get_slope(xml_node, dir));
  } else if (element->is(type::EOTD)) {
    network.element_data_[element_id].emplace<eotd>(get_eotd(xml_node));
  } else if (element->is(type::SPEED_LIMIT)) {
    network.element_data_[element_id].emplace<speed_limit>(
        get_speed_limit(xml_node, dicts, speed_limit::source::infrastructure));
  } else if (element->is(type::APPROACH_SIGNAL)) {
    network.element_data_[element_id].emplace<approach_signal>(
        get_approach_signal(xml_node));
  } else if (element->is(type::MAIN_SIGNAL)) {
    network.element_data_[element_id].emplace<main_signal>(
        get_main_signal(xml_node));
  } else if (element->is(type::BRAKE_PATH)) {
    network.element_data_[element_id].emplace<brake_path>(
        get_brake_path(xml_node, dicts));
  } else if (element->is(type::LZB_START)) {
    network.element_data_[element_id].emplace<lzb_start>(
        get_lzb_start(xml_node));
  } else if (element->is(type::LZB_BLOCK_SIGN)) {
    network.element_data_[element_id].emplace<lzb_block_sign>(
        get_lzb_block_sign(xml_node));
  }
}

element* parse_track_element(xml_node const& node, mileage_dir const dir,
                             line::id const line, dictionaries const& dicts,
                             graph& network, station& station,
                             construction_materials& mats) {
  auto element = create_element(network, station, mats, get_type(node.name()),
                                parse_rp_node_id(node));

  element->as<track_element>().dir_ = dir;

  set_line(*element, node.name(), line);
  set_km(*element, node.name(),
         parse_kilometrage(node.child_value(KILOMETER_POINT)));

  add_element_data(node, element, network, dir, dicts);

  return element;
}

}  // namespace soro::infra