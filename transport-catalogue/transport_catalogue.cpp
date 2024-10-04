#include "transport_catalogue.h"

namespace transport {

    //Хэшер для хранения в deque
    size_t StopsHasher::operator()(const std::pair<const Stop*, const Stop*> stops) const {
        return hasher(stops.first->name) + 37 * hasher(stops.second->name);
    }

    void TransportCatalogue::AddStop(std::string_view stop_name, Coordinates coords) {
        Stop stop = { std::string(stop_name), coords };
        Stop& stop_ref = stops_.emplace_back(std::move(stop));

        stop_names_.emplace(stop_ref.name, stop_ref);

        stop_to_buses_[stop_ref.name];
    }

    void TransportCatalogue::AddBusRoute(std::string_view route_name, const std::vector<std::string_view>& stop_names, bool is_circular) {
        std::vector<Stop*> route_stops;
        for (std::string_view stop_name : stop_names) {
            Stop* stop = &stop_names_.at(stop_name);
            route_stops.push_back(stop);

            stop_to_buses_[stop->name].insert(std::string(route_name));
        }

        BusRoute bus_route = { std::string(route_name), route_stops, is_circular };
        BusRoute& bus_route_ref = buses_.emplace_back(std::move(bus_route));

        bus_routes_.emplace(bus_route_ref.name, bus_route_ref);
    }

    const BusRoute* TransportCatalogue::GetBusRoute(std::string_view route_name) const {
        auto it = bus_routes_.find(route_name);
        return it != bus_routes_.end() ? &it->second : nullptr;
    }

    const Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
        auto it = stop_names_.find(stop_name);
        return it != stop_names_.end() ? &it->second : nullptr;
    }

    std::vector<std::string_view> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
        auto it = stop_to_buses_.find(stop_name);
        if (it == stop_to_buses_.end()) {
            return {};
        }

        const auto& buses = it->second;
        std::vector<std::string_view> result(buses.begin(), buses.end());

        std::sort(result.begin(), result.end());

        return result;
    }

    // Метод получения информации о маршруте
    const InfoRoute TransportCatalogue::GetInfoRoute(const BusRoute* bus_route) const {
        InfoRoute info_route;
        info_route.name = bus_route->name;
        info_route.length = GetRouteLength(*bus_route);

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
            total_length += ComputeDistance(prev_stop->coordinates, current_stop->coordinates);
        }

        return total_length;
    }

} // namespace transport
