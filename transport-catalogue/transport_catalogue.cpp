#include "transport_catalogue.h"

namespace transport {

    void TransportCatalogue::AddStop(const std::string& stop_name, Coordinates coords) {
        stops_.emplace(stop_name, Stop{ stop_name, coords });
    }

    void TransportCatalogue::AddBusRoute(const std::string& route_name, const std::vector<std::string>& stop_names, bool is_circular) {
        std::vector<Stop*> route_stops;
        for (const auto& stop_name : stop_names) {
            route_stops.push_back(&stops_.at(stop_name));
            stop_to_buses_[stop_name].insert(route_name);
        }
        bus_routes_.emplace(route_name, BusRoute{ route_name, route_stops, is_circular });
    }

    const BusRoute* TransportCatalogue::GetBusRoute(const std::string& route_name) const {
        if (bus_routes_.count(route_name)) {
            return &bus_routes_.at(route_name);
        }
        return nullptr;
    }

    const Stop* TransportCatalogue::GetStop(const std::string& stop_name) const {
        if (stops_.count(stop_name)) {
            return &stops_.at(stop_name);
        }
        return nullptr;
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

    std::vector<std::string_view> TransportCatalogue::GetBusesByStop(const std::string& stop_name) const {
        if (stop_to_buses_.count(stop_name) == 0) {
            return {};
        }

        const auto& buses = stop_to_buses_.at(stop_name);
        std::vector<std::string_view> result(buses.begin(), buses.end());

        std::sort(result.begin(), result.end());

        return result;
    }
} //namespace transport