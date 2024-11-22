#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "domain.h"
#include "map_renderer.h"
#include "json_builder.h"
#include <sstream>


namespace request_handler {

	class RequestHandler {

	public:

		RequestHandler(const map::MapRenderer& map_renderer) : map_renderer_(map_renderer) {}

		json::Dict ParseStopRequest(const transport::TransportCatalogue& catalogue, const json::Dict& request_map);

		json::Dict ParseBusRequest(const transport::TransportCatalogue& catalogue, const json::Dict& request_map);

		json::Dict ParseMapRequest(const json::Dict& request_map, map::MapRenderer& map_renderer);

		json::Array ParseStatRequests(const transport::TransportCatalogue& catalogue, const json::Array& stat_requests, map::MapRenderer& map_renderer);

	private:
		
		map::MapRenderer map_renderer_;

	};

} // namespace request_handler