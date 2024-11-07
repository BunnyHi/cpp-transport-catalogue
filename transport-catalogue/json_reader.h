#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <algorithm>

namespace json_reader {

    void ParseStop(transport::TransportCatalogue& catalogue, const json::Dict& stop_map);

    void ParseDistance(transport::TransportCatalogue& catalogue, const json::Dict& stop_map);

    void ParseBus(transport::TransportCatalogue& catalogue, const json::Dict& bus_map);

    void ParseBaseRequests(transport::TransportCatalogue& catalogue, const json::Array& base_requests);

    svg::Color ParseColor(const json::Node& color_node);

    map::RenderSettings ParseRenderSettings(const json::Dict& render_settings);

} // namespace json_reader
