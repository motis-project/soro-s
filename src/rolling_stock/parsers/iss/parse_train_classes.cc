#include "soro/rolling_stock/parsers/iss/parse_train_classes.h"

#include "pugixml.hpp"

#include "soro/base/soro_types.h"

#include "soro/utls/parse_fp.h"
#include "soro/utls/parse_int.h"

#include "soro/si/units.h"

#include "soro/infrastructure/dictionary.h"
#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

#include "soro/rolling_stock/parsers/iss/parse_stop_mode.h"
#include "soro/rolling_stock/train_class.h"

namespace soro::rs {

using namespace soro::infra;

train_class parse_train_class(pugi::xml_node const& class_xml) {
  auto const key = parse_dictionary_key<train_class::key>(
      class_xml.child_value("Abkuerzung"));

  auto const nr = utls::parse_int<train_class::nr>(class_xml.child_value("Nr"));

  auto const deacceleration = -si::from_m_s2(
      utls::parse_fp<si::accel::precision, utls::replace_comma::ON>(
          class_xml.child_value("Bremsverzoegerung")));

  auto const max_speed = si::from_km_h(
      utls::parse_fp<si::speed::precision, utls::replace_comma::ON>(
          class_xml.child_value("MaxGeschwindigkeitKonventionell")));

  auto const max_speed_ctc = si::from_km_h(
      utls::parse_fp<si::speed::precision, utls::replace_comma::ON>(
          class_xml.child_value("MaxGeschwindigkeitLZB")));

  auto const stop_mode =
      parse_stop_mode(class_xml.child("RelevanteHalteplatzArt"));

  auto const description = soro::string{class_xml.child_value("Bezeichnung")};

  return {.key_ = key,
          .nr_ = nr,
          .description_ = description,
          .deacceleration_ = deacceleration,
          .max_speed_ = max_speed,
          .max_speed_ctc_ = max_speed_ctc,
          .stop_mode_ = stop_mode};
}

soro::map<train_class::key, train_class> parse_train_classes(
    infra::iss_files const& iss_files) {
  soro::map<train_class::key, train_class> result;

  for (auto const& core_xml : iss_files.core_data_files_) {
    auto const& core_data_xml = core_xml.child(XML_ISS_DATA).child(CORE_DATA);

    auto const& train_classes_xml =
        core_data_xml.child("Konstruktionszuggruppen");

    for (auto const& train_class_xml :
         train_classes_xml.children("Konstruktionszuggruppe")) {
      auto const train_class = parse_train_class(train_class_xml);
      result.insert(soro::pair<train_class::key, struct train_class>{
          train_class.key_, train_class});
    }
  }

  return result;
}

}  // namespace soro::rs
