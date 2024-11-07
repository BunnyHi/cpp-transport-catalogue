#pragma once

#include <string>
#include <vector>
#include <variant>

#include "geo.h"

namespace transport {

    struct Stop {
        std::string name;
        geo::Coordinates coords;
    };

    struct BusRoute {
        std::string name;
        std::vector<Stop*> stops;
        int unique_stops;
        bool is_circular;
        int total_stops;
    };

    struct InfoRoute {
        size_t stops_count;
        size_t unique_stops_count;
        double length;
        double curvature;
        bool is_roundtrip;
    };

    struct InfoStop {
        std::string name;
        std::vector<std::string> buses;
    };

    struct StopsHasher {
        size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
            std::hash<const void*> ptr_hasher;
            return ptr_hasher(static_cast<const void*>(stops.first))
                + 37 * ptr_hasher(static_cast<const void*>(stops.second));
        }
    };

} // namespace transport