#include "request_handler.h"

namespace request_handler {

    json::Dict RequestHandler::ParseStopRequest(const transport::TransportCatalogue& catalogue, const json::Dict& request_map) {
        json::Builder builder;
        builder.StartDict()
            .Key("request_id").Value(request_map.at("id").AsInt());

        const std::string& stop_name = request_map.at("name").AsString();
        if (catalogue.StopExists(stop_name)) {
            if (const auto stop_info = catalogue.GetStopInfo(stop_name)) {
                builder.Key("buses").StartArray();
                for (const std::string& bus_name : stop_info->buses) {
                    builder.Value(bus_name);
                }
                builder.EndArray();
            }
        }
        else {
            builder.Key("error_message").Value("not found");
        }
        return builder.Build().AsMap();
    }

    json::Dict RequestHandler::ParseBusRequest(const transport::TransportCatalogue& catalogue, const json::Dict& request_map) {
        json::Builder builder;
        builder.StartDict()
            .Key("request_id").Value(request_map.at("id").AsInt());

        const std::string& bus_name = request_map.at("name").AsString();
        if (const auto bus_info = catalogue.GetBusInfo(bus_name)) {
            builder.Key("curvature").Value(bus_info->curvature);
            builder.Key("route_length").Value(bus_info->length);
            builder.Key("stop_count").Value(static_cast<int>(bus_info->stops_count));
            builder.Key("unique_stop_count").Value(static_cast<int>(bus_info->unique_stops_count));
        }
        else {
            builder.Key("error_message").Value("not found");
        }
        return builder.Build().AsMap();
    }

    json::Dict RequestHandler::ParseMapRequest(const json::Dict& request_map, map::MapRenderer& map_renderer) {
        json::Builder builder;
        builder.StartDict()
            .Key("request_id").Value(request_map.at("id").AsInt());

        std::ostringstream svg_stream;
        svg::Document map = map_renderer.RenderMap();
        map.Render(svg_stream);

        builder.Key("map").Value(svg_stream.str());

        return builder.Build().AsMap();
    }

    json::Array RequestHandler::ParseStatRequests(const transport::TransportCatalogue& catalogue, const json::Array& stat_requests, map::MapRenderer& map_renderer) {
        json::Builder builder;
        builder.StartArray();
        for (const auto& request : stat_requests) {
            const auto& request_map = request.AsMap();
            const auto& type = request_map.at("type").AsString();
            if (type == "Stop") {
                builder.Value(ParseStopRequest(catalogue, request_map));
            }
            else if (type == "Bus") {
                builder.Value(ParseBusRequest(catalogue, request_map));
            }
            else if (type == "Map") {
                builder.Value(ParseMapRequest(request_map, map_renderer));
            }
        }
        builder.EndArray();
        return builder.Build().AsArray();
    }

} // namespace request_handler