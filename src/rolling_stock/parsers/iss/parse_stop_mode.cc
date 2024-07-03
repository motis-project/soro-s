#include "soro/rolling_stock/parsers/iss/parse_stop_mode.h"

#include "utl/verify.h"

#include "soro/utls/sassert.h"
#include "soro/utls/string.h"

#include "soro/rolling_stock/stop_mode.h"

namespace soro::rs {

stop_mode parse_stop_mode(pugi::xml_node const& stop_mode_xml) {
  utls::sassert(utls::equal(stop_mode_xml.child_value(), "R") ||
                    utls::equal(stop_mode_xml.child_value(), "G"),
                "got {} for stop mode", stop_mode_xml.child_value());

  if (utls::equal(stop_mode_xml.child_value(), "R")) {
    return stop_mode::passenger;
  } else if (utls::equal(stop_mode_xml.child_value(), "G")) {
    return stop_mode::freight;
  }

  throw utl::fail("could not parse stop mode {}", stop_mode_xml.child_value());
}

}  // namespace soro::rs
