#include "stat_reader.h"

namespace get_stat {

    void ParseAndPrintStat(const transport::TransportCatalogue& catalogue, std::string_view request, std::ostream& output) {
        if (request.substr(0, 3) == "Bus") {
            std::string bus_name = std::string(request.substr(4));

            const transport::BusRoute* bus_route = catalogue.GetBusRoute(bus_name);

            if (bus_route) {
                const transport::InfoRoute info_route = catalogue.GetInfoRoute(bus_route);
                output << "Bus " << bus_name << ": "
                    << bus_route->stops.size() << " stops on route, "
                    << info_route.count_unique_stops << " unique stops, "
                    << info_route.length << " route length, "
                    << info_route.curvature << " curvature" << std::endl;
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