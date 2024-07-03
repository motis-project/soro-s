#pragma once

#include "soro/base/soro_types.h"

#include "soro/utls/sassert.h"
#include "soro/utls/std_wrapper/distance.h"
#include "soro/utls/std_wrapper/fill.h"
#include "soro/utls/std_wrapper/find.h"
#include "soro/utls/string.h"
#include "soro/utls/template/is_std_array.h"

#include "soro/infrastructure/brake_path.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

#include "soro/rolling_stock/train_physics.h"
#include "soro/rolling_stock/train_series.h"

namespace soro::infra {

template <typename Key>
inline Key parse_dictionary_key(char const* const val) {
  utls::expect(val != nullptr && *val != '\0', "got no dictionary key");

  Key result;
  utls::fill(result, '\0');

  utls::sassert(strlen(val) <= result.size());
  std::memcpy(result.data(), val,
              sizeof(typename Key::value_type) * strlen(val));

  return result;
}

template <typename From, typename To>
struct dictionary {
  // we really only want to translate from char arrays of keys to strong ids
  static_assert(utls::is_std_array_v<From>);
  static_assert(cista::is_strong_v<To>);

  To get_id(From const& key) const {
    auto const it = utls::find(keys_, key);

    utls::sassert(it != std::end(keys_), "cannot find key {}", key);

    return To{utls::distance<typename To::value_t>(std::begin(keys_), it)};
  }

  soro::string const& description(From const key) const {
    return description(get_id(key));
  }

  soro::string const& description(To const key) const {
    return descriptions_[key];
  }

  From const& key(To const to_key) const { return keys_[to_key]; }

  To size() const {
    return To{utls::narrow<typename To::value_t>(keys_.size())};
  }

  soro::vector_map<To, From> keys_;
  soro::vector_map<To, soro::string> descriptions_;
};

namespace detail {

template <typename From, typename To>
inline dictionary<From, To> parse_dictionary(pugi::xml_node const& dict_xml) {
  dictionary<From, To> result;

  utls::sasserts([&] {
    auto const dict_key_length = utls::parse_int<std::size_t>(
        dict_xml.child("Kurztext").attribute("Laenge").value());
    utls::sassert(std::tuple_size_v<From> >= dict_key_length);
  });

  for (auto const& key_xml :
       dict_xml.child("Schluesselwerte").children("Schluesselwert")) {
    result.keys_.emplace_back(
        parse_dictionary_key<From>(key_xml.child_value("Kurztext")));

    result.descriptions_.emplace_back(key_xml.child_value("Mitteltext"));
  }

  utls::ensure(result.keys_.size() == result.descriptions_.size());

  return result;
}

}  // namespace detail

namespace detail {

template <typename From, typename To>
dictionary<From, To> get_dictionary(iss_files const& iss_files,
                                    char const* const key_name) {
  dictionary<From, To> result;

  for (auto const& core_xml : iss_files.core_data_files_) {
    auto const& core_data_xml = core_xml.child(XML_ISS_DATA).child(CORE_DATA);

    for (auto const& dict_xml : core_data_xml.child("Schluesselwerttypen")
                                    .children("Schluesselwerttyp")) {
      if (utls::equal(dict_xml.child_value(NAME), key_name)) {
        return parse_dictionary<From, To>(dict_xml);
      }
    }
  }

  throw utl::fail("could not find dictionary for {} key", key_name);
}

}  // namespace detail

using brake_path_lengths =
    soro::vector_map<brake_path, soro::optional<si::length>>;

struct dictionaries {

  dictionary<brake_path_key, brake_path> brake_path_;
  dictionary<rs::brake_position_key, rs::brake_position> brake_position_;
  dictionary<rs::brake_type_key, rs::brake_type> brake_type_;
  dictionary<rs::tilt_technology_key, rs::tilt_technology> tilt_technology_;
  dictionary<rs::train_series::type_key, rs::train_series::type>
      train_series_type_;
  dictionary<rs::transportation_specialty_key, rs::transportation_specialty>
      transportation_specialty_;
  dictionary<rs::electric_configuration_key, rs::electric_configuration_t>
      electric_configuration_;

  soro::map<rs::brake_position, rs::brake_type> brake_position_to_brake_type_;

  brake_path_lengths brake_path_length_;
};

dictionaries get_dictionaries(iss_files const& iss_files);

}  // namespace soro::infra
