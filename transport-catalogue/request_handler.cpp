#include "request_handler.h"

namespace request_handler {

    json::Dict RequestHandler::ParseStopRequest(const transport::TransportCatalogue& catalogue, const json::Dict& request_map) {
        json::Dict response;
        response["request_id"] = request_map.at("id").AsInt();
        const std::string& stop_name = request_map.at("name").AsString();
        if (catalogue.StopExists(stop_name)) {
            if (const auto stop_info = catalogue.GetStopInfo(stop_name)) {
                json::Array buses;
                for (const std::string& bus_name : stop_info->buses) {
                    buses.push_back(json::Node{ bus_name });
                }
                response["buses"] = json::Node{ buses };
            }
        }
        else {
            response["error_message"] = "not found";
        }
        return response;
    }

    json::Dict RequestHandler::ParseBusRequest(const transport::TransportCatalogue& catalogue, const json::Dict& request_map) {
        json::Dict response;
        response["request_id"] = request_map.at("id").AsInt();
        const std::string& bus_name = request_map.at("name").AsString();
        if (const auto bus_info = catalogue.GetBusInfo(bus_name)) {
            response["curvature"] = bus_info->curvature;
            response["route_length"] = bus_info->length;
            response["stop_count"] = static_cast<int>(bus_info->stops_count);
            response["unique_stop_count"] = static_cast<int>(bus_info->unique_stops_count);
        }
        else {
            response["error_message"] = "not found";
        }
        return response;
    }

    json::Dict RequestHandler::ParseMapRequest(const json::Dict& request_map, map::MapRenderer& map_renderer) {
        json::Dict response;
        response["request_id"] = request_map.at("id").AsInt();

        std::ostringstream svg_stream;
        svg::Document map = map_renderer.RenderMap();
        map.Render(svg_stream);

        response["map"] = svg_stream.str();

        return response;
    }

    json::Array RequestHandler::ParseStatRequests(const transport::TransportCatalogue& catalogue, const json::Array& stat_requests, map::MapRenderer& map_renderer) {
        json::Array responses;
        for (const auto& request : stat_requests) {
            const auto& request_map = request.AsMap();
            const auto& type = request_map.at("type").AsString();
            if (type == "Stop") {
                responses.push_back(ParseStopRequest(catalogue, request_map));
            }
            else if (type == "Bus") {
                responses.push_back(ParseBusRequest(catalogue, request_map));
            }
            else if (type == "Map") {
                responses.push_back(ParseMapRequest(request_map, map_renderer));
            }
        }
        return responses;
    }

}