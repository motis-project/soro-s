#include "soro/rolling_stock/parsers/iss/parse_train_type.h"

#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/rolling_stock/train_type.h"

namespace soro::rs {

train_type parse_train_type(pugi::xml_node const& train_type_xml) {
  utls::sassert(utls::equal(train_type_xml.child_value(), "R") ||
                    utls::equal(train_type_xml.child_value(), "G"),
                "got {} for train type", train_type_xml.child_value());

  return utls::equal(train_type_xml.child_value(), "R")
             ? rs::train_type::passenger
             : rs::train_type::freight;
}

}  // namespace soro::rs