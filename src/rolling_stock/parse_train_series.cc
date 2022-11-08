#include "soro/rolling_stock/parse_train_series.h"

#include "pugixml.hpp"

#include "soro/utls/parse_fp.h"

#include "soro/si/constants.h"

namespace soro::rs {

using namespace pugi;
using namespace soro::utls;

// --- factory methods for parsing into SI units -- //

// --- for tractive forces ---

auto from_kilo_newton(si::precision const kilo_newton) {
  // does kN -> N
  return tractive_force_1_t{kilo_newton * 1000};
}

auto from_kn_h_per_km(si::precision const kn_h_per_km) {
  // does [kN * h / km] -> [N * s / m]
  return tractive_force_2_t{kn_h_per_km * 1000 * 3.6};
}

auto from_kn_h2_per_km2(si::precision const kn_h2_per_km2) {
  // does [kN * h² / km²] -> [N * s / m]
  return tractive_force_3_t{kn_h2_per_km2 * 1000 * 3.6 * 3.6};
}

// --- for running resistances --- //

inline auto from_unitless_in_per_mille(si::precision const unitless) {
  // does not have any units and is given in per mille
  return si::unitless{unitless / 1000.0};
}

inline auto from_h_km(si::precision const h_km) {
  // does [h / km] -> [s / m], given in per mille
  return decltype(std::declval<si::time>() /
                  std::declval<si::length>()){(h_km * 3.6) / 1000.0};
}

inline auto from_kg_h_km(si::precision const kg_h_km) {
  // does [kg * h² / km²] -> [kg * s² / m²]
  return decltype((std::declval<si::weight>() * std::declval<si::time>() *
                   std::declval<si::time>()) /
                  (std::declval<si::length>() * std::declval<si::length>())){
      kg_h_km * std::pow(3.6, 2)};
}

tractive_curve_t get_tractive_force_curve(
    xml_node const& tractive_force_factors) {
  soro::vector<tractive_piece_t> polynomials;

  for (auto const& factor_tuple :
       tractive_force_factors.children("Zugkraftfaktor")) {

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

  return make_piecewise(std::move(polynomials));
}

auto get_running_resistance_curve(xml_node const& running_resistance_factors,
                                  si::weight const train_weight) {
  auto base_rolling =
      from_unitless_in_per_mille(parse_fp<si::precision, replace_comma::ON>(
          running_resistance_factors.child("Laufwiderstandsfaktor1")
              .child_value()));

  auto base_dampening = from_h_km(parse_fp<si::precision, replace_comma::ON>(
      running_resistance_factors.child("Laufwiderstandsfaktor2")
          .child_value()));

  auto base_drag = from_kg_h_km(parse_fp<si::precision, replace_comma::ON>(
      running_resistance_factors.child("Laufwiderstandsfaktor3")
          .child_value()));

  rolling_resistance_t rolling =
      base_rolling * si::GRAVITATIONAL * train_weight;
  dampening_resistance_t dampening =
      base_dampening * si::GRAVITATIONAL * train_weight;
  drag_coefficient_t drag = base_drag * si::GRAVITATIONAL;

  return utls::make_polynomial(drag, dampening, rolling);
}

auto parse_variants(xml_node const& xml_variants) {
  soro::map<variant_id, traction_vehicle> variants;

  for (auto const& xml_variant :
       xml_variants.children("Triebfahrzeugbaureihenvariante")) {

    variant_id const id =
        static_cast<uint32_t>(std::stoul(xml_variant.child_value("Variante")));

    auto weight = si::from_ton(parse_fp<si::precision, replace_comma::ON>(
        xml_variant.child("EigenGewicht").child_value()));

    variants.emplace(
        id, traction_vehicle{
                .name_ = xml_variant.child("Bezeichnung").child_value(),
                .weight_ = weight,
                .max_speed_ =
                    si::from_km_h(parse_fp<si::precision, replace_comma::ON>(
                        xml_variant.child("ZulaessigeGeschwindigkeit")
                            .child_value())),
                .tractive_curve_ = get_tractive_force_curve(
                    xml_variant.child("Stromartausruestungen")
                        .child("Stromartausruestung")
                        .child("Zugkraftfaktoren")),
                .resistance_curve_ =
                    get_running_resistance_curve(xml_variant, weight)});
  }

  return variants;
}

soro::vector<train_series> parse_train_series(
    xml_node const& xml_train_model_ranges) {
  soro::vector<train_series> train_series;

  for (auto xml_train_model_range :
       xml_train_model_ranges.children("Triebfahrzeugbaureihe")) {
    struct train_series ts;

    ts.nr_ = xml_train_model_range.child_value("Nr");

    ts.company_nr_ = xml_train_model_range.child("Eisenbahnunternehmen")
                         .attribute("Nr")
                         .value();

    ts.variants_ = parse_variants(
        xml_train_model_range.child("Triebfahrzeugbaureihenvarianten"));

    train_series.push_back(ts);
  }

  return train_series;
}

}  // namespace soro::rs
