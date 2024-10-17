#include "transport_catalogue.h"

namespace transport {

    size_t StopsHasher::operator()(const std::pair<const Stop*, const Stop*>& stops) const {
        std::hash<const void*> ptr_hasher;
        return ptr_hasher(static_cast<const void*>(stops.first))
            + 37 * ptr_hasher(static_cast<const void*>(stops.second));
    }

    void TransportCatalogue::AddStop(std::string_view stop_name, Coordinates coords) {
        Stop& stop_ref = stops_.emplace_back(Stop{ std::string(stop_name), coords });

        stop_names_.emplace(stop_ref.name, &stop_ref);

        stop_to_buses_[stop_ref.name];
    }

    void TransportCatalogue::AddBusRoute(std::string_view route_name, const std::vector<std::string_view>& stop_names, bool is_circular) {
        BusRoute& bus_route_ref = buses_.emplace_back(BusRoute{ std::string(route_name), {}, is_circular });
        std::vector<Stop*> route_stops;

        for (std::string_view stop_name : stop_names) {
            Stop* stop = stop_names_.at(stop_name);
            route_stops.push_back(stop);

            stop_to_buses_[stop->name].insert(bus_route_ref.name);
        }

        bus_route_ref.stops = std::move(route_stops);

        bus_routes_.emplace(bus_route_ref.name, &bus_route_ref);
    }

    const BusRoute* TransportCatalogue::GetBusRoute(std::string_view route_name) const {
        auto it = bus_routes_.find(route_name);
        return it != bus_routes_.end() ? it->second : nullptr;
    }

    const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
        auto it = stop_names_.find(stop_name);
        return it != stop_names_.end() ? it->second : nullptr;
    }

    std::vector<std::string_view> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
        if (stop_to_buses_.count(stop_name) == 0) {
            return {};
        }

        const auto& buses = stop_to_buses_.at(stop_name);
        std::vector<std::string_view> result(buses.begin(), buses.end());

        std::sort(result.begin(), result.end());

        return result;
    }

    const InfoRoute TransportCatalogue::GetInfoRoute(const BusRoute* bus_route) const {
        InfoRoute info_route;
        info_route.name = bus_route->name;
        info_route.length = GetRouteLength(*bus_route);

        double geo_length = 0.0;
        for (size_t i = 1; i < bus_route->stops.size(); ++i) {
            const Stop* prev_stop = bus_route->stops[i - 1];
            const Stop* current_stop = bus_route->stops[i];

            geo_length += ComputeDistance(prev_stop->coordinates, current_stop->coordinates);
        }
        info_route.curvature = info_route.length / geo_length;

        std::unordered_set<std::string_view> unique_stops;
        for (const Stop* stop : bus_route->stops) {
            unique_stops.insert(stop->name);
        }

        info_route.count_unique_stops = static_cast<int>(unique_stops.size());

        return info_route;
    }

    double TransportCatalogue::GetRouteLength(const BusRoute& bus_route) const {
        double total_length = 0.0;

        for (size_t i = 1; i < bus_route.stops.size(); ++i) {
            const Stop* prev_stop = bus_route.stops[i - 1];
            const Stop* current_stop = bus_route.stops[i];

            total_length += GetDistance(prev_stop, current_stop);
        }

        return total_length;
    }

    void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, double distance) {
        distances_[{from, to}] = distance;
    }

    double TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
        if (distances_.count({ from, to })) {
            return distances_.at({ from, to });
        }
        if (distances_.count({ to, from })) {
            return distances_.at({ to, from });
        }
        return 0.0;
    }

} // namespace transport
