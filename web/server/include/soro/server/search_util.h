#pragma once

#include <string>
#include <vector>


namespace soro::server {

    enum class osm_type {
        HALT,
        STATION,
        MAIN_SIGNAL,

        UNDEFINED,
    };

    struct osm_object {
        public:
        osm_object() = default;
          osm_object(const std::string& name, const osm_type type,
                   const double lon,
           const double lat)
           : name_(name), type_(type), lon_(lon), lat_(lat) {}

        std::string name_;
        osm_type type_; 
        double lon_;
        double lat_;
    };


    struct search_filter {
    public:
        search_filter() = default;
        bool station_ = false;
        bool halt_ = false;
        bool main_signal_ = false;
    };


    std::string map_type(const osm_type type);

    std::string to_lower(std::string str);

    std::vector<soro::server::osm_object> get_object_info(const std::vector<soro::server::osm_object>& osm_objects, const std::string& name, const soro::server::search_filter& filter);

}  // namespace soro::server