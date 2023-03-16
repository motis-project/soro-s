#include "doctest/doctest.h"

#include "soro/server/search_util.h"

#include <algorithm>

using namespace soro::server;

TEST_SUITE_BEGIN("osm search suite");

std::vector<soro::server::osm_object> generate_objects() {
  std::vector<soro::server::osm_object> objects;

  objects.emplace_back("Frankfurt", osm_type::STATION, 50, 34);
  objects.emplace_back("Darmstadt", osm_type::STATION, 64, 42);
  objects.emplace_back("Darmstadt Nord", osm_type::STATION, 15, 40);
  objects.emplace_back("Weiterstadt", osm_type::HALT, 23, 78);
  objects.emplace_back("F", osm_type::MAIN_SIGNAL, 50, 35);

  return objects;
}

std::vector<std::string> map_names(
    const std::vector<soro::server::osm_object>& objects) {
  std::vector<std::string> names;

  std::for_each(
      objects.begin(), objects.end(),
      [&names](const osm_object& obj) { names.push_back(obj.name_);
  });

  return names;
}

TEST_CASE("to_lower") { 
	CHECK(to_lower("test") == "test");
	CHECK(to_lower("Test") == "test");
    CHECK(to_lower("TEST") == "test");
    CHECK(to_lower("") == "");
}

TEST_CASE("map_type") { 
	CHECK(map_type(osm_type::HALT) == "hlt");
    CHECK(map_type(osm_type::STATION) == "station");
    CHECK(map_type(osm_type::MAIN_SIGNAL) == "ms");
    CHECK(map_type(osm_type::UNDEFINED) == "undefined");
}

TEST_CASE("get_object_info_1") { 
    auto const objects = generate_objects();
    
    auto const filter = search_filter();

    auto const result = get_object_info(objects, "F", filter);

    CHECK(result.size() == 0);
}

TEST_CASE("get_object_info_2") {
    auto const objects = generate_objects();

    auto filter = search_filter();
    filter.station_ = true;
    filter.main_signal_ = true;

    auto const result = get_object_info(objects, "F", filter);

    CHECK(result.size() == 2);

    auto names = map_names(result);

    CHECK(std::find(names.begin(), names.end(), "Frankfurt") != names.end());
    CHECK(std::find(names.begin(), names.end(), "F") != names.end());
}


TEST_CASE("get_object_info_3") {
    auto const objects = generate_objects();

    auto filter = search_filter();
    filter.station_ = true;
    filter.halt_ = true;

    auto const result = get_object_info(objects, "stadt", filter);

    CHECK(result.size() == 3);

    auto names = map_names(result);

    CHECK(std::find(names.begin(), names.end(), "Darmstadt") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Darmstadt Nord") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Weiterstadt") !=names.end());
}


TEST_CASE("get_object_info_4") {
    auto const objects = generate_objects();

    auto filter = search_filter();
    filter.station_ = true;
    filter.halt_ = true;

    auto const result = get_object_info(objects, "", filter);

    CHECK(result.size() == 4);

    auto names = map_names(result);

    CHECK(std::find(names.begin(), names.end(), "Darmstadt") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Darmstadt Nord") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Weiterstadt") != names.end());
    CHECK(std::find(names.begin(), names.end(), "Frankfurt") != names.end());
}


TEST_SUITE_END();