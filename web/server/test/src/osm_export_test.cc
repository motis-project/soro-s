#include "doctest/doctest.h"

#include "soro/utls/string.h"

#include "soro/base/fp_precision.h"
#include "soro/infrastructure/infrastructure.h"
#include "soro/server/osm_export/osm_export.h"

#include "test/file_paths.h"

using namespace soro;
using namespace soro::infra;
using namespace soro::server::osm_export;

TEST_SUITE_BEGIN("osm export suite");  // NOLINT

void check_node_ids(pugi::xml_node osm_node) {
  std::size_t last_id = 0;
  for (pugi::xml_node node = osm_node.first_child(); node != nullptr;
       node = node.next_sibling()) {
    std::size_t const id = std::stoul(node.first_attribute().value());
    CHECK(id + 1 > last_id);
    last_id = id;
  }
}

void check_contains_elements_stations(pugi::xml_node osm_node,
                                      auto const& iss) {
  std::vector<element_id> element_ids;
  std::vector<station::id> station_ids;

  for (pugi::xml_node node = osm_node.first_child(); node != nullptr;
       node = node.next_sibling()) {
    if (strcmp(node.first_child().first_attribute().next_attribute().value(),
               "element") == 0) {
      element_id const id =
          static_cast<element_id>(std::stoul(node.first_child()
                                                 .next_sibling()
                                                 .next_sibling()
                                                 .first_attribute()
                                                 .next_attribute()
                                                 .value()));
      element_ids.push_back(id);
    } else if (strcmp(node.first_child()
                          .first_attribute()
                          .next_attribute()
                          .value(),
                      "station") == 0) {
      station::id const id = node.first_child()
                                 .next_sibling()
                                 .first_attribute()
                                 .next_attribute()
                                 .as_uint();
      station_ids.push_back(id);
    }
  }

  // --- Check stations --- //

  // every infrastructure station must correspond to an osm station ...
  CHECK(station_ids.size() == iss.stations_.size());

  // ... no duplicate station ids allowed
  std::sort(station_ids.begin(), station_ids.end());
  bool const duplicate_stations =
      std::adjacent_find(station_ids.begin(), station_ids.end()) !=
      station_ids.end();
  CHECK(!duplicate_stations);

  // --- Check elements --- //

  // osm element ids must not contain duplicates
  std::sort(element_ids.begin(), element_ids.end());
  bool const duplicate_element =
      std::adjacent_find(element_ids.begin(), element_ids.end()) !=
      element_ids.end();
  CHECK(!duplicate_element);

  // every element id from the osm file must appear in the infrastructure
  for (auto id : element_ids) {
    CHECK(iss.element_to_station_.find(id) !=
          std::end(iss.element_to_station_));
  }

  // every element id must appear in the osm file
  for (auto const& [e_id, station_ptr] : iss.element_to_station_) {
    CHECK(utls::contains(element_ids, e_id));
  }

  for (auto const& pair : iss.element_to_station_) {
    auto id = pair.first;
    CHECK(std::find(element_ids.begin(), element_ids.end(), id) !=
          element_ids.end());
  }
}

void check_ways(auto const& element, auto const& ids, auto neighbours,
                auto const& element_to_station) {
  CHECK(ids.contains(element->id()));
  auto id = ids.at(element->id());
  for (auto neighbour : neighbours) {
    if (neighbour != nullptr && element_to_station.at(element->id()) ==
                                    element_to_station.at(neighbour->id())) {
      CHECK(std::find(id.begin(), id.end(), neighbour->id()) != id.end());
    }
  }
}
void check_ways_between_elements(pugi::xml_node osm_node, auto const& iss) {
  std::map<element_id, std::vector<element_id>> ids;
  for (pugi::xml_node tag = osm_node.first_child(); tag != nullptr;
       tag = tag.next_sibling()) {
    std::string const name = tag.name();
    if (strcmp(tag.name(), "way") != 0) {
      continue;
    }
    if (std::distance(tag.children().begin(), tag.children().end()) > 3) {
      continue;
    }

    auto const first = static_cast<element_id>(
        std::stoul(tag.first_child().first_attribute().value()));
    auto const second = static_cast<element_id>(
        std::stol(tag.first_child().next_sibling().first_attribute().value()));
    if (ids.contains(first)) {
      ids.at(first).push_back(second);
    } else {
      std::vector<element_id> id;
      id.push_back(second);
      ids[first] = id;
    }
    if (ids.contains(second)) {
      ids.at(second).push_back(first);
    } else {
      std::vector<element_id> id;
      id.push_back(first);
      ids[second] = id;
    }
  }

  for (auto station : iss.stations_) {
    for (auto element : station->elements_) {
      element->e_.apply([&](auto&& e) {
        check_ways(element, ids, e.neighbours_, iss.element_to_station_);
      });
    }
  }
}

void check_key_value(auto const& key, auto const& value, auto const& node,
                     auto const& id) {  // NOLINT
  auto attribute = node.first_attribute();
  CHECK(attribute != nullptr);
  for (int i = 0; i < id; i++) {
    attribute = attribute.next_attribute();
    CHECK(attribute != nullptr);
  }
  std::string const name = attribute.name();
  std::string const val = attribute.value();
  CHECK(name == key);
  CHECK(val == value);
}

void check_tag(auto const& key, auto const& value, auto const& node) {
  std::string const name = node.name();
  CHECK(name == "tag");
  check_key_value("k", key, node, 0);
  check_key_value("v", value, node, 1);
}

void check_nd(auto const& number, auto const& node) {
  std::string const name = node.name();
  CHECK(name == "nd");
  check_key_value("ref", number, node, 0);
}

void check_node_id(auto const& name, auto const& id, auto const& node) {
  std::string const node_name = node.name();
  CHECK(node_name == name);
  check_key_value("id", id, node, 0);
}

void check_way_duplicates(pugi::xml_node parent_node) {
  std::map<element_id, std::vector<element_id>> ids;
  for (pugi::xml_node tag = parent_node.first_child(); tag != nullptr;
       tag = tag.next_sibling()) {
    if (strcmp(tag.name(), "way") != 0) {
      continue;
    }
    if (std::distance(tag.children().begin(), tag.children().end()) > 3) {
      continue;
    }

    auto const first = static_cast<element_id>(
        std::stoul(tag.first_child().first_attribute().value()));
    auto const second = static_cast<element_id>(
        std::stol(tag.first_child().next_sibling().first_attribute().value()));
    if (ids.contains(first)) {
      auto vec = ids.at(first);
      // if found then there exists a duplicate
      if (std::find(vec.begin(), vec.end(), second) != vec.end()) {
        CHECK(first == second);
      }
      vec.push_back(second);
      ids[first] = vec;
    } else {
      std::vector const vec = {second};
      ids[first] = vec;
    }
    if (ids.contains(second)) {
      auto vec = ids.at(second);
      vec.push_back(first);
      ids[second] = vec;
    } else {
      std::vector const vec = {first};
      ids[second] = vec;
    }
  }
}

TEST_CASE("osm_check") {  // NOLINT
  infrastructure const infra(soro::test::SMALL_OPTS);
  pugi::xml_document const osm_file = export_to_osm(*infra);

  auto osm_node = osm_file.document_element();
  CHECK(utls::equal(osm_node.name(), "osm"));

  check_node_ids(osm_node);
  check_contains_elements_stations(osm_node, *infra);
  check_ways_between_elements(osm_node, *infra);
  check_way_duplicates(osm_node);
}

TEST_CASE("check_osm_station_generation") {  // NOLINT
  infrastructure const infra(soro::test::SMALL_OPTS);
  auto station = infra->stations_[1];

  pugi::xml_document testfile;
  auto node = testfile.append_child("test_node");
  detail::create_station_osm(node, station, 1, infra->station_positions_);

  auto id_at = node.first_child().first_attribute();
  std::string const id_name = id_at.name();
  CHECK(id_name == "id");
  CHECK(std::stoi(id_at.value()) == 1);

  auto const coords = infra->station_positions_[station->id_];

  auto lon_at = node.first_child().first_attribute().next_attribute();
  std::string name = lon_at.name();
  CHECK(name == "lon");
  std::string val = lon_at.value();
  CHECK(equal(std::stod(val), coords.lon_));
  auto lat_at =
      node.first_child().first_attribute().next_attribute().next_attribute();
  name = lat_at.name();
  CHECK(name == "lat");
  val = lat_at.value();
  CHECK(equal(std::stod(val), coords.lat_));

  auto tag = node.first_child().first_child();
  check_tag("type", "station", tag);

  tag = node.first_child().first_child().next_sibling();
  check_tag("id", std::to_string(station->id_), tag);

  tag = node.first_child().first_child().next_sibling().next_sibling();
  check_tag("name", station->ds100_, tag);
}

TEST_CASE("check_osm_element_generation") {  // NOLINT
  infrastructure const infra(soro::test::SMALL_OPTS);
  auto station = infra->stations_.front();
  auto element = station->elements_.front();
  pugi::xml_document testfile;
  auto node = testfile.append_child("test_node");
  detail::create_element_osm(node, element, infra->element_positions_);

  check_node_id("node", std::to_string(element->id()), node.first_child());

  auto tag = node.first_child().first_child();
  check_tag("type", "element", tag);

  tag = node.first_child().first_child().next_sibling();
  check_tag("subtype", element->get_type_str(), tag);

  tag = node.first_child().first_child().next_sibling().next_sibling();
  check_tag("id", std::to_string(element->id()), tag);
}

TEST_CASE("check_osm_way_generation") {  // NOLINT
  pugi::xml_document testfile;
  auto node = testfile.append_child("test_node");
  detail::create_way_osm(node, 1, 2, 3);

  check_node_id("way", std::to_string(3), node.first_child());

  auto nd = node.first_child().first_child();
  check_nd(std::to_string(1), nd);

  nd = node.first_child().first_child().next_sibling();
  check_nd(std::to_string(2), nd);

  auto tag = node.first_child().first_child().next_sibling().next_sibling();
  check_tag("railway", "rail", tag);
}
bool compare_string_alternatives(auto string_compare, auto a, auto b) {
  return strcmp(string_compare, a) == 0 || strcmp(string_compare, b) == 0;
}

TEST_SUITE_END();  // NOLINT
