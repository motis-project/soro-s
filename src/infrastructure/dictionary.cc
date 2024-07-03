#include "soro/infrastructure/dictionary.h"

#include <cctype>
#include <algorithm>
#include <string>

#include "soro/base/soro_types.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/all_of.h"
#include "soro/utls/std_wrapper/count.h"
#include "soro/utls/string.h"

#include "soro/si/units.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

#include "soro/rolling_stock/train_physics.h"
#include "soro/rolling_stock/train_series.h"

namespace soro::infra {

using namespace rs;

soro::map<brake_position, brake_type> get_brake_position_to_brake_type(
    iss_files const& iss_files,
    dictionary<brake_position_key, brake_position> const& brake_positions,
    dictionary<brake_type_key, brake_type> const& brake_types) {
  soro::map<brake_position, brake_type> result;

  for (auto const& core_xml : iss_files.core_data_files_) {
    auto const& map_xml =
        core_xml.child(XML_ISS_DATA).child(CORE_DATA).child("Bremsarten");

    for (auto const& entry_xml : map_xml.children("Bremsart")) {
      auto const pos_val =
          entry_xml.child("BremsstellungZug").attribute(KEY).value();
      auto const pos_key = parse_dictionary_key<brake_position_key>(pos_val);
      auto const pos_id = brake_positions.get_id(pos_key);

      auto const type_val =
          entry_xml.child("BremsartKennzeichen").attribute(KEY).value();
      auto const type_key = parse_dictionary_key<brake_type_key>(type_val);
      auto const type_id = brake_types.get_id(type_key);

      result.emplace(pos_id, type_id);
    }
  }

  return result;
}

soro::vector_map<brake_path, soro::optional<si::length>> get_brake_path_length(
    dictionary<brake_path_key, brake_path> const& brake_path_dict) {
  soro::vector_map<brake_path, soro::optional<si::length>> result;

  for (brake_path path{0}; path < brake_path_dict.size(); ++path) {
    auto const& desc = brake_path_dict.description(path);
    auto const begin = desc.data();
    auto const end = desc.data() + desc.size();

    auto const first_digit = std::find_if(begin, end, ::isdigit);

    if (first_digit == end) {
      result.emplace_back(soro::optional<si::length>{});
      continue;
    }

    auto const after_last_digit = std::find_if(
        first_digit, end, [](auto&& c) { return !std::isdigit(c); });

    auto const length =
        utls::parse_fp<si::length::precision>(first_digit, after_last_digit);

    result.emplace_back(si::from_m(length));
  }

  return result;
}

dictionary<transportation_specialty_key, transportation_specialty>
parse_transportation_specialties_dict(iss_files const& iss_files) {
  dictionary<transportation_specialty_key, transportation_specialty> result;

  for (auto const& core_xml : iss_files.core_data_files_) {
    auto const specialties_xml = core_xml.child(XML_ISS_DATA)
                                     .child(CORE_DATA)
                                     .child("Beschreibungsvorlagen");

    if (!static_cast<bool>(specialties_xml)) {
      continue;
    }

    for (auto const& specialty_xml : specialties_xml.children()) {
      // ignore everything except "BefBesG", as these appear in speed limits
      if (!utls::equal(specialty_xml.child_value("Fachname"), "BefBesG")) {
        continue;
      }

      auto const key = parse_dictionary_key<transportation_specialty_key>(
          specialty_xml.child_value("KurzBezeichnung"));

      result.keys_.emplace_back(key);
      result.descriptions_.emplace_back(
          specialty_xml.child_value("Textvorlage"));
    }
  }

  utls::ensure(utls::all_of(result.keys_,
                            [&](auto&& key) {
                              return utls::count(result.keys_, key) == 1;
                            }),
               "no double keys");

  return result;
}

dictionaries get_dictionaries(iss_files const& iss_files) {
  dictionaries result;

  result.brake_path_ = detail::get_dictionary<brake_path_key, brake_path>(
      iss_files, "BREMSWEG-ART");
  result.brake_position_ =
      detail::get_dictionary<rs::brake_position_key, rs::brake_position>(
          iss_files, "BREMSSTELLUNG-ZUG");
  result.brake_type_ =
      detail::get_dictionary<rs::brake_type_key, rs::brake_type>(
          iss_files, "BREMSART-KENNZEICHEN");
  result.tilt_technology_ =
      detail::get_dictionary<rs::tilt_technology_key, rs::tilt_technology>(
          iss_files, "NEIGUNGSTECHNIK");
  result.train_series_type_ = detail::get_dictionary<rs::train_series::type_key,
                                                     rs::train_series::type>(
      iss_files, "TRIEBFAHRZEUG-ART");
  result.electric_configuration_ =
      detail::get_dictionary<rs::electric_configuration_key,
                             rs::electric_configuration_t>(iss_files,
                                                           "STROMART");
  result.transportation_specialty_ =
      parse_transportation_specialties_dict(iss_files);

  result.brake_position_to_brake_type_ = get_brake_position_to_brake_type(
      iss_files, result.brake_position_, result.brake_type_);

  result.brake_path_length_ = get_brake_path_length(result.brake_path_);

  return result;
}

}  // namespace soro::infra