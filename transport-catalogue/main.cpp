#include <iostream>
#include "json.h"
#include "json_reader.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

int main() {

    transport::TransportCatalogue catalogue;
    json::Document doc = json::Load(std::cin);
    const auto& root = doc.GetRoot().AsMap();

    const auto& base_requests = root.at("base_requests").AsArray();
    json_reader::ParseBaseRequests(catalogue, base_requests);

    const auto& render_settings = root.at("render_settings").AsMap();
    const auto& render = json_reader::ParseRenderSettings(render_settings);

    map::MapRenderer map_renderer(render, catalogue);

    const auto& routing_settings = root.at("routing_settings").AsMap();
    const auto& router_set = json_reader::ParseRouterSettings(routing_settings);

    transport::Router router{ router_set, catalogue };

    const auto& stat_requests = root.at("stat_requests").AsArray();
    request_handler::RequestHandler request_handler(map_renderer);

    json::Array responses = request_handler.ParseStatRequests(catalogue, stat_requests, map_renderer, router);
    json::Document response_doc{ std::move(responses) };
    json::Print(response_doc, std::cout);

}
