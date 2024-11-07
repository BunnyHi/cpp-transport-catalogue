#include "json_reader.h"

namespace json_reader {

    void ParseStop(transport::TransportCatalogue& catalogue, const json::Dict& stop_map) {
        const std::string& name = stop_map.at("name").AsString();
        const auto& latitude = stop_map.at("latitude").AsDouble();
        const auto& longitude = stop_map.at("longitude").AsDouble();

        geo::Coordinates coords{ latitude, longitude };
        catalogue.AddStop(name, coords);
    }

    void ParseDistance(transport::TransportCatalogue& catalogue, const json::Dict& stop_map) {
        const std::string& name = stop_map.at("name").AsString();
        const transport::Stop* current_stop = catalogue.GetStopByName(name);

        if (stop_map.count("road_distances")) {
            const auto& road_distances = stop_map.at("road_distances").AsMap();

            for (const auto& [other_stop_name, distance_node] : road_distances) {
                double distance = distance_node.AsDouble();
                const transport::Stop* other_stop = catalogue.GetStopByName(other_stop_name);
                catalogue.SetRoadDistance(current_stop, other_stop, distance);
            }
        }
    }

    void ParseBus(transport::TransportCatalogue& catalogue, const json::Dict& bus_map) {
        const std::string& name = bus_map.at("name").AsString();
        const auto& stops = bus_map.at("stops").AsArray();
        bool is_roundtrip = bus_map.at("is_roundtrip").AsBool();

        std::vector<std::string_view> stop_names;
        for (const auto& stop_node : stops) {
            stop_names.push_back(stop_node.AsString());
        }

        catalogue.AddBus(name, stop_names, is_roundtrip);
    }

    void ParseBaseRequests(transport::TransportCatalogue& catalogue, const json::Array& base_requests) {
        for (const auto& request : base_requests) {
            const auto& type = request.AsMap().at("type").AsString();

            if (type == "Stop") {
                ParseStop(catalogue, request.AsMap());
            }
        }

        for (const auto& request : base_requests) {
            const auto& type = request.AsMap().at("type").AsString();

            if (type == "Stop") {
                ParseDistance(catalogue, request.AsMap());
            }
        }

        for (const auto& request : base_requests) {
            const auto& type = request.AsMap().at("type").AsString();

            if (type == "Bus") {
                ParseBus(catalogue, request.AsMap());
            }
        }
    }

    svg::Color ParseColor(const json::Node& color_node) {
        if (color_node.IsString()) {
            return color_node.AsString();
        }
        else if (color_node.IsArray()) {
            const json::Array& color_array = color_node.AsArray();
            if (color_array.size() == 3) {
                return svg::Rgb(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt());
            }
            else if (color_array.size() == 4) {
                return svg::Rgba(color_array[0].AsInt(), color_array[1].AsInt(), color_array[2].AsInt(), color_array[3].AsDouble());
            }
        }
        throw std::logic_error("Invalid color format");
    }

    map::RenderSettings ParseRenderSettings(const json::Dict& render_settings) {
        map::RenderSettings settings;

        settings.width = render_settings.at("width").AsDouble();
        settings.height = render_settings.at("height").AsDouble();
        settings.padding = render_settings.at("padding").AsDouble();
        settings.stop_radius = render_settings.at("stop_radius").AsDouble();
        settings.line_width = render_settings.at("line_width").AsDouble();
        settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsInt();

        const json::Array& bus_label_offset = render_settings.at("bus_label_offset").AsArray();
        settings.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };

        settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsInt();
        const json::Array& stop_label_offset = render_settings.at("stop_label_offset").AsArray();
        settings.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

        settings.underlayer_color = ParseColor(render_settings.at("underlayer_color"));
        settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();

        const json::Array& color_palette = render_settings.at("color_palette").AsArray();
        for (const auto& color_element : color_palette) {
            settings.color_palette.push_back(ParseColor(color_element));
        }

        return settings;
    }

} // namespace json_reader
