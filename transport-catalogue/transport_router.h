#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

namespace transport {

    struct RouteItem {
        std::string stop_name;
        double time;
        std::string type;
        std::string bus_name;
        int span_count;
    };

    struct Route {
        double total_time;
        std::vector<RouteItem> items;
    };

    class Router {
    public:
        Router() = default;
        Router(int bus_wait_time, double bus_velocity);
        Router(const Router& settings, const TransportCatalogue& catalogue);

        const graph::DirectedWeightedGraph<double>& BuildGraph(const TransportCatalogue& catalogue);
        const std::optional<Route> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
        const graph::DirectedWeightedGraph<double>& GetGraph() const;

    private:
        void AddStopsToGraph(const std::map<std::string_view, const Stop*>& stops_map, graph::DirectedWeightedGraph<double>& transport_graph);
        void AddBusesToGraph(const std::map<std::string_view, const BusRoute*>& buses_map, graph::DirectedWeightedGraph<double>& transport_graph, const TransportCatalogue& catalogue);
        double CalculateDistanceBetweenStops(const TransportCatalogue& catalogue, const std::vector<Stop*>& stops, size_t from_index, size_t to_index);
        double CalculateDistanceBetweenStopsInverse(const TransportCatalogue& catalogue, const std::vector<Stop*>& stops, size_t from_index, size_t to_index);
        void AddBusRouteToGraph(const BusRoute* bus_info, graph::DirectedWeightedGraph<double>& transport_graph, const Stop* stop_from, const Stop* stop_to, double distance, size_t stop_count);

        int bus_wait_time_ = 0;
        double bus_velocity_ = 0.0;

        graph::DirectedWeightedGraph<double> graph_;
        std::unordered_map<std::string, graph::VertexId> stop_vertex_ids_;
        std::unique_ptr<graph::Router<double>> router_;
    };

} // namespace transport 