#include "soro/rolling_stock/parsers/iss/parse_train_categories.h"

#include <iterator>

#include "utl/logging.h"
#include "utl/timer.h"

#include "soro/base/soro_types.h"

#include "soro/utls/parse_int.h"

#include "soro/infrastructure/parsers/iss/iss_files.h"
#include "soro/rolling_stock/parsers/iss/parse_train_type.h"
#include "soro/rolling_stock/train_category.h"

#include "soro/infrastructure/parsers/iss/iss_string_literals.h"

namespace soro::rs {

using namespace soro::infra;

soro::vector<train_category> parse_train_category(
    pugi::xml_node const& train_main_category_xml) {
  soro::vector<train_category> result;

  auto const main_val =
      train_main_category_xml.child_value(TRAIN_CATEGORY_MAIN_NUMBER);
  auto const main = train_category::key::main{
      utls::parse_int<train_category::key::main::value_t>(main_val)};

  auto const train_categories_xml =
      train_main_category_xml.child(TRAIN_CATEGORIES);

  for (auto const train_category_xml :
       train_categories_xml.children(TRAIN_CATEGORY)) {
    train_category tc;
    tc.key_.main_ = main;

    auto const sub_val =
        train_category_xml.child_value(TRAIN_CATEGORY_SUB_NUMBER);
    tc.key_.sub_ = train_category::key::sub{
        utls::parse_int<train_category::key::sub::value_t>(sub_val)};

    tc.key_.name_ = train_category_xml.child_value("ZuggattungsProdukt");

    tc.main_description_ = train_main_category_xml.child_value("Beschreibung");
    tc.sub_description_ =
        train_category_xml.child_value("ZuggattungsunternummernBezeichner");

    tc.type_ = parse_train_type(train_category_xml.child("ZugartFV"));

    result.push_back(tc);
  }

  return result;
}

soro::map<train_category::key, train_category> parse_train_categories(
    iss_files const& iss_files) {
  utl::scoped_timer const timer("parsing train categories");

  soro::map<train_category::key, train_category> result;

  for (auto const& core_xml : iss_files.core_data_files_) {
    auto const& core_data_xml = core_xml.child(XML_ISS_DATA).child(CORE_DATA);

    auto const& train_categories_xml =
        core_data_xml.child(TRAIN_MAIN_CATEGORIES);

    for (auto const train_category_xml :
         train_categories_xml.children(TRAIN_MAIN_CATEGORY)) {

      auto const cats = parse_train_category(train_category_xml);
      for (auto const& c : cats) {
        auto it = result.find(c.key_);

        // TODO(julian) why is this key not unique?
        // utl::verify(it == std::end(result), "overwriting a train category!");
        if (it != std::end(result)) {
          uLOG(utl::warn) << "overwriting train category: " << c.key_.main_
                          << "." << c.key_.sub_ << " " << c.key_.name_;
        }

        result[c.key_] = c;
      }
    }
  }

  return result;
}

}  // namespace soro::rs