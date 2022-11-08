#include "emscripten/bind.h"

#include "soro/infrastructure/infrastructure.h"

#include "soro/web/register_vector.h"

using namespace soro;
using namespace soro::infra;
using namespace soro::utls;

element const& get_element(node const& n) { return *n.element_; }

element_id get_element_id(element const& e) { return e.id(); }

type get_element_type(element const& e) { return e.type(); }

track_element const& as_track_element(element const& e) { return e.template as<track_element>(); }

element const& element_from_infra(infrastructure const& i,
                                  std::size_t const id) {
  return *(i->graph_.elements_[id]);
}

station::ptr const& get_station(station_route const& sr) { return sr.station_; }

auto const& get_stations(infrastructure const& i) {
  return i->stations_;
}

auto const& get_station_coords(infrastructure const& i) {
  return i->station_positions_;
}

auto const& get_element_coords(infrastructure const& i) {
  return i->element_positions_;
}

auto const& get_station_routes(infrastructure const& i) {
  return i->station_routes_;
}

auto const& get_signal_station_routes(
    infrastructure const& i) {
  return i->interlocking_.interlocking_routes_;
}

auto const& get_station_to_ssrs(infrastructure const& i) {
  return i->interlocking_.station_to_irs_;
}

auto get_network(infrastructure const& i) {
  return &(i->graph_);
}

EMSCRIPTEN_BINDINGS(gps) {
  emscripten::class_<gps>("GPS")
      .property("lat", &gps::lat_)
      .property("lon", &gps::lon_);

  emscripten::register_vector<gps>("GPSList");
}

auto get_station_route_id(station_route const& sr) {
  return sr.id_;
}

auto get_station_route_nodes(station_route const& sr) {
  return sr.nodes();
}

EMSCRIPTEN_BINDINGS(station_route) {
  emscripten::class_<station_route>("StationRoute")
      .property("id", &get_station_route_id)
      .property("nodes", &get_station_route_nodes)
      .property("name", &station_route::name_)
      .property("station", &get_station);

  emscripten::register_map<std::string, station_route::ptr>(
      "StrStationRouteMap");

  emscripten::register_vector<station_route::ptr>("StationRoutePtrList");
}

auto get_interlocking_route_id(interlocking_route const& ir) {
  return ir.id_;
}

auto get_interlocking_route_nodes(interlocking_route const& ir) {
  return ir.nodes();
}

EMSCRIPTEN_BINDINGS(interlocking_route) {
  emscripten::class_<interlocking_route>("SignalStationRoute")
      .property("id", &get_interlocking_route_id)
      .property("nodes", &get_interlocking_route_nodes);

  emscripten::register_vector<interlocking_route::ptr>(
      "SignalStationRoutePtrList");

  emscripten::function("validSSRID", &interlocking_route::valid);
}

EMSCRIPTEN_BINDINGS(infrastructure_options) {
  emscripten::class_<infrastructure_options>("InfrastructureOptions")
      .constructor<>()
      .property("determine_conflicts",
                &infrastructure_options::determine_conflicts_)
      .property("infrastructure_path",
                &infrastructure_options::infrastructure_path_)
      .property("gps_coord_path", &infrastructure_options::gps_coord_path_);
}

EMSCRIPTEN_BINDINGS(network) {
  emscripten::class_<graph>("InfrastructureGraph");
}

EMSCRIPTEN_BINDINGS(iss) {
  emscripten::enum_<type>("ElementType")
      .value("BUMPER", type::BUMPER)
      .value("TRACK_END", type::TRACK_END)
      .value("KM_JUMP", type::KM_JUMP)
      .value("BORDER", type::BORDER)
      .value("LINE_SWITCH", type::LINE_SWITCH)
      .value("SIMPLE_SWITCH", type::SIMPLE_SWITCH)
      .value("CROSS", type::CROSS)
      .value("MAIN_SIGNAL", type::MAIN_SIGNAL)
      .value("PROTECTION_SIGNAL", type::PROTECTION_SIGNAL)
      .value("APPROACH_SIGNAL", type::APPROACH_SIGNAL)
      .value("RUNTIME_CHECKPOINT", type::RUNTIME_CHECKPOINT)
      .value("EOTD", type::EOTD)
      .value("SPEED_LIMIT", type::SPEED_LIMIT)
      .value("CTC", type::CTC)
      .value("HALT", type::HALT)
      .value("TUNNEL", type::TUNNEL)
      .value("SLOPE", type::SLOPE)
      .value("INVALID", type::INVALID);

  emscripten::class_<element>("Element")
      .property("id", &get_element_id)
      .property("type", &get_element_type)
      .function("is_track_element", &element::is_track_element)
      .function("as_track_element", &as_track_element);

  emscripten::class_<track_element>("TrackElement")
      .property("rising", &track_element::rising_)
      .property("km", &track_element::km_);

  emscripten::class_<node>("Node")
      .property("id", &node::id_)
      .property("element", &get_element);

  emscripten::register_vector<node_ptr>("NodePtrList");

  emscripten::class_<station>("Station")
      .property("id", &station::id_)
      .property("name", &station::ds100_)
      .property("station_routes", &station::station_routes_);

  emscripten::register_map<std::string, station::ptr>("StrStationMap");

  register_vector<station::ptr>("StationPtrList");
  register_vector<soro::vector<interlocking_route::ptr>>(
      "InterlockingRouteListList");

  emscripten::class_<infrastructure>("Infrastructure")
      .constructor<infrastructure_options>()
      .function("save", &infrastructure::save)
      .property("stations", &get_stations)
      .property("station_coords", &get_station_coords)
      .property("element_coords", &get_element_coords)
      .property("station_routes", &get_station_routes)
      .property("signal_station_routes", &get_signal_station_routes)
      .property("station_to_ssrs", &get_station_to_ssrs)
      .function("element", &element_from_infra)
      .function("network", &get_network, emscripten::allow_raw_pointers());
}
