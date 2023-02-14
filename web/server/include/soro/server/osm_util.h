#pragma once

#include <string>


namespace soro::server {

struct osm_halt {
public:
    osm_halt() = default;
    osm_halt(const std::string& name, const bool is_station, const double lon,
           const double lat)
      : name_(name), is_station_(is_station), lon_(lon), lat_(lat) {}

    std::string name_;
    bool is_station_; // true if halt is a station, false if halt is a stop
    double lon_;
    double lat_;
};

}  // namespace soro::server