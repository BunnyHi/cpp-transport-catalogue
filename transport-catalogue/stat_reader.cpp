#include "stat_reader.h"

namespace get_stat {

    void ParseAndPrintStat(const transport::TransportCatalogue& catalogue, std::string_view request, std::ostream& output) {
        if (request.substr(0, 3) == "Bus") {
            std::string bus_name = std::string(request.substr(4));

            const transport::BusRoute* bus_route = catalogue.GetBusRoute(bus_name);

            if (bus_route) {
                std::unordered_set<std::string_view> unique_stops;
                for (const transport::Stop* stop : bus_route->stops) {
                    unique_stops.insert(stop->name);
                }

                double route_length = catalogue.GetRouteLength(*bus_route);

                output << "Bus " << bus_name << ": "
                    << bus_route->stops.size() << " stops on route, "
                    << unique_stops.size() << " unique stops, "
                    << route_length << " route length" << std::endl;
            }
            else {
                output << "Bus " << bus_name << ": not found" << std::endl;
            }
        }

        else if (request.substr(0, 5) == "Stop ") {
            std::string stop_name = std::string(request.substr(5));
            const transport::Stop* stop = catalogue.GetStop(stop_name);

            if (!stop) {
                output << "Stop " << stop_name << ": not found" << std::endl;
                return;
            }

            std::vector<std::string_view> buses = catalogue.GetBusesByStop(stop_name);

            if (buses.empty()) {
                output << "Stop " << stop_name << ": no buses" << std::endl;
            }
            else {
                output << "Stop " << stop_name << ": buses";
                for (const auto& bus : buses) {
                    output << " " << bus;
                }
                output << std::endl;
            }
        }
    }
} //namespace get_stat