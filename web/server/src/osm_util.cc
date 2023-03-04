#include "soro/server/osm_util.h"

namespace soro::server {
    std::string map_type(const osm_type type) {
        switch (type) {
            case osm_type::HALT: return "hlt";
            case osm_type::STATION: return "station";
            case osm_type::MAIN_SIGNAL: return "ms";
            default: return "undefined";
        }
    }
}  // namespace soro::server