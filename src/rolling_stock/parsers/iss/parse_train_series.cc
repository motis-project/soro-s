#include "soro/rolling_stock/parsers/iss/parse_train_series.h"

#include <cmath>
#include <utility>
#include <vector>

#include "pugixml.hpp"

#include "utl/logging.h"
#include "utl/timer.h"
#include "utl/verify.h"

#include "soro/base/soro_types.h"

#include "soro/utls/math/piecewise_function.h"
#include "soro/utls/math/polynomial.h"
#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/string.h"

#include "soro/si/constants.h"
#include "soro/si/units.h"

#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

#include "soro/rolling_stock/safety_systems.h"
#include "soro/rolling_stock/train_series.h"

namespace soro::rs {

using namespace pugi;
using namespace soro::utls;
using namespace soro::infra;

// --- factory methods for parsing into SI units -- //

// --- for tractive forces ---

tractive_force_1_t from_kilo_newton(si::precision const kilo_newton) {
  // does kN -> N
  return {kilo_newton * 1000};
}

tractive_force_2_t from_kn_h_per_km(si::precision const kn_h_per_km) {
  // does [kN * h / km] -> [N * s / m]
  return {kn_h_per_km * 1000 * 3.6};
}

tractive_force_3_t from_kn_h2_per_km2(si::precision const kn_h2_per_km2) {
  // does [kN * h² / km²] -> [N * s / m]
  return {kn_h2_per_km2 * 1000 * 3.6 * 3.6};
}

// --- for running resistances --- //

si::unitless from_unitless_in_per_mille(si::precision const unitless) {
  // does not have any units and is given in per mille
  return {unitless / 1000.0};
}

auto from_h_km(si::precision const h_km) {
  // does [h / km] -> [s / m], given in per mille
  return decltype(std::declval<si::time>() /
                  std::declval<si::length>()){(h_km * 3.6) / 1000.0};
}

auto from_kg_h_km(si::precision const kg_h_km) {
  // does [kg * h² / km²] -> [kg * s² / m²]
  return decltype((std::declval<si::weight>() * std::declval<si::time>() *
                   std::declval<si::time>()) /
                  (std::declval<si::length>() * std::declval<si::length>())){
      kg_h_km * std::pow(3.6, 2)};
}

traction_unit::equipment get_equipment(xml_node const equipment_xml,
                                       dictionaries const& dicts) {
  traction_unit::equipment result;

  soro::vector<tractive_piece_t> polynomials;

  for (auto const& factor_tuple :
       equipment_xml.child("Zugkraftfaktoren").children("Zugkraftfaktor")) {

    tractive_piece_t::input_t const from =
        si::from_km_h(parse_fp<si::precision, replace_comma::ON>(
            factor_tuple.child("GeschwindigkeitVon").child_value()));
    tractive_piece_t::input_t const to =
        si::from_km_h(parse_fp<si::precision, replace_comma::ON>(
            factor_tuple.child("GeschwindigkeitBis").child_value()));

    tractive_force_1_t factor1 =
        from_kilo_newton(parse_fp<si::precision, replace_comma::ON>(
            factor_tuple.child("Faktor1").child_value()));
    tractive_force_2_t factor2 =
        from_kn_h_per_km(parse_fp<si::precision, replace_comma::ON>(
            factor_tuple.child("Faktor2").child_value()));
    tractive_force_3_t factor3 =
        from_kn_h2_per_km2(parse_fp<si::precision, replace_comma::ON>(
            factor_tuple.child("Faktor3").child_value()));

    tractive_polynomial_t const polynomial =
        make_polynomial(factor3, factor2, factor1);  // NOLINT
    tractive_piece_t const piece = make_piece(polynomial, from, to);

    polynomials.push_back(piece);
  }

  if (!is_continuous(polynomials)) {
    uLOG(utl::warn) << "Found non continuous piecewise function for tractive "
                       "force curve, skipping it.";
    return {};
  }

  result.curve_ = make_piecewise(std::move(polynomials));

  auto const config_key = parse_dictionary_key<electric_configuration_key>(
      equipment_xml.child("Stromart").attribute("Schluessel").value());

  // no current is modeled as no value in the electric current optional
  if (config_key != electric_configuration_key{"0"}) {
    utls::sassert(dicts.electric_configuration_.description(config_key) !=
                  "ohne Stromart");
    result.current_.emplace(dicts.electric_configuration_.get_id(config_key));
  }

  return result;
}

soro::vector_map<traction_unit::equipment::idx, traction_unit::equipment>
get_equipments(xml_node const& equipments_xml, dictionaries const& dicts) {
  soro::vector_map<traction_unit::equipment::idx, traction_unit::equipment>
      result;

  for (auto const equip_xml : equipments_xml.children("Stromartausruestung")) {
    auto const equipment = get_equipment(equip_xml, dicts);

    if (equipment.curve_.pieces_.empty()) return {};

    result.push_back(equipment);
  }

  return result;
}

resistance_curve_t get_running_resistance_curve(
    xml_node const& running_resistance_factors, si::weight const train_weight) {
  auto const base_rolling =
      from_unitless_in_per_mille(parse_fp<si::precision, replace_comma::ON>(
          running_resistance_factors.child("Laufwiderstandsfaktor1")
              .child_value()));

  auto const base_dampening =
      from_h_km(parse_fp<si::precision, replace_comma::ON>(
          running_resistance_factors.child("Laufwiderstandsfaktor2")
              .child_value()));

  auto const base_drag =
      from_kg_h_km(parse_fp<si::precision, replace_comma::ON>(
          running_resistance_factors.child("Laufwiderstandsfaktor3")
              .child_value()));

  rolling_resistance_t rolling =
      base_rolling * si::GRAVITATIONAL * train_weight;
  dampening_resistance_t dampening =
      base_dampening * si::GRAVITATIONAL * train_weight;
  drag_coefficient_t drag = base_drag * si::GRAVITATIONAL;

  return utls::make_polynomial(drag, dampening, rolling);
}

si::unitless from_base_mass_factor(
    si::unitless::precision const base_mass_factor) {
  return si::unitless{1.0} + si::unitless{base_mass_factor / 100.0};
}

soro::pair<soro::string, si::weight> get_brake_weight(
    xml_node const& brake_weight) {
  return {brake_weight.child("Bremsstellung").attribute("Schluessel").value(),
          si::from_ton(utls::parse_fp<si::weight::precision>(
              brake_weight.child_value("Gewicht")))};
}

soro::map<soro::string, si::weight> get_brake_weights(
    xml_node const& brake_weights) {
  soro::map<soro::string, si::weight> result;

  for (auto const& brake_weight : brake_weights.children("Bremsgewicht")) {
    result.insert(get_brake_weight(brake_weight));
  }

  return result;
}

soro::map<train_series::variant, traction_unit> parse_variants(
    xml_node const& xml_variants, train_series::key const& tsk,
    train_series::type const series_type, dictionaries const& dicts) {
  soro::map<train_series::variant, traction_unit> variants;

  for (auto const& xml_variant :
       xml_variants.children("Triebfahrzeugbaureihenvariante")) {
    traction_unit::key tuk;

    tuk.train_series_key_.number_ = tsk.number_;
    tuk.train_series_key_.company_ = tsk.company_;
    tuk.variant_ =
        train_series::variant{utls::parse_int<train_series::variant::value_t>(
            xml_variant.child_value("Variante"))};

    auto const weight = si::from_ton(parse_fp<si::precision, replace_comma::ON>(
        xml_variant.child("EigenGewicht").child_value()));

    auto const length =
        si::from_m(parse_fp<si::length::precision, replace_comma::ON>(
            xml_variant.child("LaengeUeberPuffer").child_value()));

    auto const base_mass_factor = (parse_fp<si::precision, replace_comma::ON>(
        xml_variant.child("MasseZuschlag").child_value()));

    auto const mass_factor = from_base_mass_factor(base_mass_factor);

    auto equipments =
        get_equipments(xml_variant.child("Stromartausruestungen"), dicts);

    if (equipments.empty()) {
      uLOG(utl::warn) << "skipping traction vehicle from series " << tsk.number_
                      << ", owner " << tsk.company_ << ", and variant "
                      << as_val(tuk.variant_);
      continue;
    }

    auto const max_speed =
        si::from_km_h(parse_fp<si::speed::precision, replace_comma::ON>(
            xml_variant.child("ZulaessigeGeschwindigkeit").child_value()));

    utls::sassert(
        utls::all_of(
            equipments,
            [&](auto&& e) { return e.curve_.pieces_.back().to_ == max_speed; }),
        "curves must be specified up to the max speed");

    auto const pzb = static_cast<PZB>(
        equal(xml_variant.child_value("PunktfoermigeZugbeeinflussung"), "Ja"));

    auto const lzb = static_cast<LZB>(
        equal(xml_variant.child_value("LinienfoermigeZugbeeinflussung"), "Ja"));

    variants.emplace(
        tuk.variant_,
        traction_unit{.key_ = tuk,
                      .type_ = series_type,
                      .name_ = xml_variant.child("Bezeichnung").child_value(),
                      .weight_ = weight,
                      .length_ = length,
                      .mass_factor_ = mass_factor,
                      .max_speed_ = max_speed,
                      .pzb_ = pzb,
                      .lzb_ = lzb,
                      .equipments_ = std::move(equipments),
                      .resistance_curve_ =
                          get_running_resistance_curve(xml_variant, weight),
                      .brake_weights_ = get_brake_weights(
                          xml_variant.child("Bremsgewichte"))});
  }

  return variants;
}

soro::vector<train_series> parse_train_series(
    xml_node const& xml_train_model_ranges, dictionaries const& dicts) {
  soro::vector<train_series> result;

  for (auto xml_train_model_range :
       xml_train_model_ranges.children("Triebfahrzeugbaureihe")) {
    train_series ts;

    auto const number_val = xml_train_model_range.child_value("Nr");
    ts.key_.number_ = train_series::key::number{number_val};

    auto const company_val = xml_train_model_range.child("Eisenbahnunternehmen")
                                 .attribute("Nr")
                                 .value();
    ts.key_.company_ = train_series::key::company{company_val};

    auto const type_key = parse_dictionary_key<train_series::type_key>(
        xml_train_model_range.child("Triebfahrzeugart")
            .attribute("Schluessel")
            .value());
    ts.type_ = dicts.train_series_type_.get_id(type_key);

    ts.variants_ = parse_variants(
        xml_train_model_range.child("Triebfahrzeugbaureihenvarianten"), ts.key_,
        ts.type_, dicts);

    result.push_back(ts);
  }

  return result;
}

soro::map<train_series::key, train_series> parse_train_series(
    iss_files const& iss_files, dictionaries const& dicts) {
  utl::scoped_timer const timer("parsing train series");

  soro::map<train_series::key, train_series> result;

  for (auto const& core_xml : iss_files.core_data_files_) {
    auto const& core_data_xml = core_xml.child(XML_ISS_DATA).child(CORE_DATA);

    auto const& train_series_xml = core_data_xml.child(TRAIN_SERIES);
    if (static_cast<bool>(train_series_xml)) {
      auto const& train_series =
          rs::parse_train_series(train_series_xml, dicts);

      for (auto const& ts : train_series) {
        auto it = result.find(ts.key_);
        utl::verify(it == std::end(result), "overwriting a train series!");

        result[ts.key_] = ts;
      }
    }
  }

  return result;
}

}  // namespace soro::rs
