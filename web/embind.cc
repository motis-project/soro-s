#include <sstream>
#include <string>

#include "emscripten/bind.h"

#include "soro/ascii_network_parser.h"
#include "soro/graphviz_output.h"
#include "soro/propagator.h"
#include "soro/route_train_order.h"
#include "soro/timetable_parser.h"

using namespace soro;

std::string simulate(std::string const& network_input,
                     std::string const& trains_input,
                     std::string const& timetable_input) {
  auto const net = parse_network(network_input);
  auto const tt = parse_timetable(net, trains_input, timetable_input);
  auto const route_train_order = compute_route_train_order(tt);
  propagate(route_train_order);

  std::stringstream ss;
  graphiz_output(ss, tt);

  return ss.str();
}

EMSCRIPTEN_BINDINGS(module) { emscripten::function("soros", &simulate); }