#include "transport_router.h"

namespace transport {

    Router::Router(int bus_wait_time, double bus_velocity)
        : bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity) {
    }

    Router::Router(const Router& settings, const TransportCatalogue& catalogue)
        : bus_wait_time_(settings.bus_wait_time_), bus_velocity_(settings.bus_velocity_) {
        BuildGraph(catalogue);
    }

    const graph::DirectedWeightedGraph<double>& Router::BuildGraph(const TransportCatalogue& catalogue) {
        const auto& stops_map = catalogue.GetSortedStops();
        const auto& buses_map = catalogue.GetSortedBuses();
        graph::DirectedWeightedGraph<double> transport_graph(stops_map.size() * 2);
        stop_vertex_ids_.clear();

        AddStopsToGraph(stops_map, transport_graph);
        AddBusesToGraph(buses_map, transport_graph, catalogue);

        graph_ = std::move(transport_graph);
        router_ = std::make_unique<graph::Router<double>>(graph_);

        return graph_;
    }

    void Router::AddStopsToGraph(const std::map<std::string_view, const Stop*>& stops_map, graph::DirectedWeightedGraph<double>& transport_graph) {
        graph::VertexId vertex_id = 0;

        for (const auto& [stop_name, stop_info] : stops_map) {
            stop_vertex_ids_[stop_info->name] = vertex_id;

            graph::VertexId current_vertex_id = vertex_id;
            vertex_id++;

            transport_graph.AddEdge({
                stop_info->name,
                0,
                current_vertex_id,
                vertex_id,
                static_cast<double>(bus_wait_time_)
                });

            vertex_id++;
        }
    }

    void Router::AddBusesToGraph(const std::map<std::string_view, const BusRoute*>& buses_map, graph::DirectedWeightedGraph<double>& transport_graph, const TransportCatalogue& catalogue) {
        for (const auto& [bus_name, bus_info] : buses_map) {
            const auto& stops = bus_info->stops;
            size_t stops_count = stops.size();

            for (size_t i = 0; i < stops_count; ++i) {
                for (size_t j = i + 1; j < stops_count; ++j) {
                    const Stop* stop_from = stops[i];
                    const Stop* stop_to = stops[j];
                    double dist_sum = CalculateDistanceBetweenStops(catalogue, stops, i, j);
                    double dist_sum_inverse = CalculateDistanceBetweenStopsInverse(catalogue, stops, i, j);

                    AddBusRouteToGraph(bus_info, transport_graph, stop_from, stop_to, dist_sum, j - i);

                    if (!bus_info->is_circular) {
                        AddBusRouteToGraph(bus_info, transport_graph, stop_to, stop_from, dist_sum_inverse, j - i);
                    }
                }
            }
        }
    }

    double Router::CalculateDistanceBetweenStops(const TransportCatalogue& catalogue, const std::vector<Stop*>& stops, size_t from_index, size_t to_index) {
        double distance = 0.0;
        for (size_t k = from_index + 1; k <= to_index; ++k) {
            distance += catalogue.GetDistance(stops[k - 1], stops[k]);
        }
        return distance / (bus_velocity_ * (100.0 / 6.0));
    }

    double Router::CalculateDistanceBetweenStopsInverse(const TransportCatalogue& catalogue, const std::vector<Stop*>& stops, size_t from_index, size_t to_index) {
        double distance = 0.0;
        for (size_t k = to_index; k > from_index; --k) {
            distance += catalogue.GetDistance(stops[k], stops[k - 1]);
        }
        return distance / (bus_velocity_ * (100.0 / 6.0));
    }

    void Router::AddBusRouteToGraph(const BusRoute* bus_info, graph::DirectedWeightedGraph<double>& transport_graph, const Stop* stop_from, const Stop* stop_to, double distance, size_t stop_count) {
        transport_graph.AddEdge({
            bus_info->name,
            stop_count,
            stop_vertex_ids_.at(stop_from->name) + 1,
            stop_vertex_ids_.at(stop_to->name),
            distance
            });
    }

    const std::optional<Route> Router::FindRoute(const std::string_view stop_from, const std::string_view stop_to) const {
        auto route_info = router_->BuildRoute(stop_vertex_ids_.at(std::string(stop_from)), stop_vertex_ids_.at(std::string(stop_to)));

        if (!route_info) {
            return std::nullopt;
        }

        Route route;
        route.total_time = 0.0;

        for (const auto& edge_id : route_info->edges) {
            const auto edge = graph_.GetEdge(edge_id);
            RouteItem item;

            if (edge.quality == 0) {
                item.stop_name = edge.name;
                item.time = edge.weight;
                item.type = "Wait";
                item.bus_name = "";
                item.span_count = 0;
            }
            else {
                item.stop_name = edge.name;
                item.time = edge.weight;
                item.type = "Bus";
                item.bus_name = edge.name;
                item.span_count = static_cast<int>(edge.quality);
            }

            route.items.push_back(item);
            route.total_time += edge.weight;
        }

        return route;
    }

    const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
        return graph_;
    }

} // namespace transport