#include "transport_catalogue.h"

namespace transport {

    void TransportCatalogue::AddStop(std::string_view stop_name, geo::Coordinates coords) {
        stops_.emplace_back(Stop{ std::string(stop_name), coords });
        stop_names_.emplace(stop_name, &stops_.back());
    }

    void TransportCatalogue::AddBus(std::string_view route_name, const std::vector<std::string_view>& stop_names, bool is_circular) {
        BusRoute route{ std::string(route_name), {}, 0, is_circular, 0 };
        BusRoute route_one_trip{ std::string(route_name), {}, 0, is_circular, 0 };
        std::unordered_set<Stop*> unique_stops_set;

        for (const auto& stop_name : stop_names) {
            auto it = stop_names_.find(stop_name);
            if (it != stop_names_.end()) {
                Stop* stop = it->second;
                route.stops.push_back(stop);
                unique_stops_set.insert(stop);
                stop_to_buses_[stop_name].insert(route_name);
            }
        }

        route.unique_stops = unique_stops_set.size();

        route.total_stops = route.stops.size();

        buses_.emplace_back(route);
        bus_routes_.emplace(route_name, &buses_.back());

    }

    bool TransportCatalogue::StopExists(std::string_view name) const {
        return std::any_of(stops_.begin(), stops_.end(),
            [&name](const Stop& stop) { return stop.name == name; });
    }

    bool TransportCatalogue::BusExists(std::string_view name) const {
        return std::any_of(buses_.begin(), buses_.end(),
            [&name](const BusRoute& bus) { return bus.name == name; });
    }

    const Stop* TransportCatalogue::GetStopByName(std::string_view name) const {
        auto it = stop_names_.find(name);
        if (it != stop_names_.end()) {
            return it->second;
        }
        return nullptr;
    }

    const BusRoute* TransportCatalogue::GetBusByName(std::string_view name) const {
        auto it = bus_routes_.find(name);
        if (it != bus_routes_.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::string> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
        std::vector<std::string> buses;
        auto it = stop_to_buses_.find(stop_name);
        if (it != stop_to_buses_.end()) {
            for (const auto& bus : it->second) {
                buses.push_back(std::string(bus));
            }
        }
        return buses;
    }

    void TransportCatalogue::SetRoadDistance(const Stop* stopA, const Stop* stopB, double distance) {
        if (stopA && stopB) {
            auto dist_pair = std::make_pair(stopA, stopB);
            distances_.insert(DistanceMap::value_type(dist_pair, distance));
        }
    }

    std::map<std::string_view, const BusRoute*> TransportCatalogue::GetSortedBuses() const {
        std::map<std::string_view, const BusRoute*> sorted_buses;
        for (const auto& route : bus_routes_) {
            sorted_buses.emplace(route);
        }
        return sorted_buses;
    }

    std::optional<InfoStop> TransportCatalogue::GetStopInfo(std::string_view stop_name) const {
        auto it = stop_to_buses_.find(stop_name);
        InfoStop info;
        info.name = stop_name;
        if (it != stop_to_buses_.end()) {
            info.buses = std::vector<std::string>(it->second.begin(), it->second.end());
            std::sort(info.buses.begin(), info.buses.end());
            return info;
        }
        else {
            info.buses = {};
            return info;
        }
        return info;
    }

    std::optional<InfoRoute> TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
        InfoRoute info = {0, 0, 0.0, 0.0, false};
        auto it = bus_routes_.find(bus_name);
        if (it != bus_routes_.end()) {
            const BusRoute& route = *(it->second);
            if (route.is_circular) {
                info.stops_count = route.stops.size();
            }
            else {
                info.stops_count = route.stops.size() * 2 - 1;
            }
            info.unique_stops_count = route.unique_stops;
            info.is_roundtrip = route.is_circular;

            int length = 0;
            double length_geo = 0.0;
            for (size_t i = 0; i < route.stops.size() - 1; ++i) {
                auto from = route.stops[i];
                auto to = route.stops[i + 1];

                if (route.is_circular) {
                    length += GetDistance(from, to);
                    length_geo += geo::ComputeDistance(from->coords, to->coords);
                }
                else {
                    length += GetDistance(from, to) + GetDistance(to, from);
                    length_geo += geo::ComputeDistance(from->coords, to->coords) * 2;
                }
            }
            info.length = length;
            info.curvature = length / length_geo;
            return info;
        }
        return std::nullopt;
    }

    double TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
        auto it = distances_.find(std::make_pair(from, to));
        if (it != distances_.end()) {
            return it->second;
        }
        else {
            return distances_.at(std::make_pair(to, from));
        }
        return 0.0;
    }

} // namespace transport