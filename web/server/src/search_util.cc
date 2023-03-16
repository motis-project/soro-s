#include "soro/server/search_util.h"

#include <algorithm>

namespace soro::server {
    std::string map_type(const osm_type type) {
        switch (type) {
            case osm_type::HALT: return "hlt";
            case osm_type::STATION: return "station";
            case osm_type::MAIN_SIGNAL: return "ms";
            default: return "undefined";
        }
    }


    std::string to_lower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return str;
    }


    std::vector<soro::server::osm_object> get_object_info(
        const std::vector<soro::server::osm_object>& osm_objects,
        const std::string& name, const soro::server::search_filter& filter) {

        std::vector<soro::server::osm_object> matches;

        using ot = soro::server::osm_type;

        for (const auto& object : osm_objects) {
            if (!filter.halt_ && object.type_ == ot::HALT) continue;
            if (!filter.station_ && object.type_ == ot::STATION) continue;
            if (!filter.main_signal_ && object.type_ == ot::MAIN_SIGNAL)
              continue;

            if (to_lower(object.name_).find(to_lower(name)) !=
                std::string::npos) {
              matches.push_back(object);
            }
        }

        std::sort(matches.begin(), matches.end(),
                  [](const soro::server::osm_object& a,
                     const soro::server::osm_object& b) {
                    // Primary ordering by length
                    if (a.name_.length() < b.name_.length()) return true;
                    if (a.name_.length() > b.name_.length()) return false;
                    // Secondary ordering by lexicographical order
                    return a.name_ < b.name_;
                  });

        return matches;
    }

}  // namespace soro::server