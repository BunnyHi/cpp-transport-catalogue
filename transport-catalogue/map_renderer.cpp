#include "map_renderer.h"

namespace map {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Document MapRenderer::RenderMap() const {
        std::map<std::string_view, const transport::BusRoute*> sorted_bus = catalogue_.GetSortedBuses();
        std::map<std::string_view, const transport::Stop*> sorted_stops;
        for (const auto& [route_name, bus_route] : sorted_bus) {
            for (const auto* stop : bus_route->stops) {
                sorted_stops[stop->name] = stop;
            }
        }

        return CreateSVGDocument(sorted_bus, sorted_stops);
    }

    svg::Document MapRenderer::CreateSVGDocument(const std::map<std::string_view, const transport::BusRoute*>& sorted_bus, const std::map<std::string_view, const transport::Stop*> sorted_stops) const {
        svg::Document doc;

        std::vector<geo::Coordinates> route_stops_coords;
        for (const auto& [bus_name, route] : sorted_bus) {
            for (const auto& stop : route->stops) {
                route_stops_coords.push_back(stop->coords);
            }
        }

        SphereProjector sphere_projector(route_stops_coords.begin(), route_stops_coords.end(),
            render_settings_.width, render_settings_.height, render_settings_.padding);

        auto polylines = CreatePolylines(sorted_bus, sphere_projector);
        for (const auto& polyline : polylines) {
            doc.Add(polyline);
        }

        auto route_names = CreateRouteNames(sorted_bus, sphere_projector);
        for (const auto& route_name : route_names) {
            doc.Add(route_name);
        }

        auto stop_symbols = CreateStopSymbols(sorted_stops, sphere_projector);
        for (const auto& symbol : stop_symbols) {
            doc.Add(symbol);
        }

        auto stop_names = CreateStopsNames(sorted_stops, sphere_projector);
        for (const auto& stop_name : stop_names) {
            doc.Add(stop_name);
        }

        return doc;
    }

    std::vector<svg::Polyline> MapRenderer::CreatePolylines(const std::map<std::string_view, const transport::BusRoute*>& sorted_bus, const SphereProjector& sphere_projector) const {
        size_t color_num = 0;
        std::vector<svg::Polyline> polylines;
        for (const auto& [bus_name, route] : sorted_bus) {
            if (route->stops.empty()) {
                continue;
            }

            svg::Polyline polyline;

            std::vector<const transport::Stop*> route_stops{ route->stops.begin(), route->stops.end() };
            if (route->is_circular == false) route_stops.insert(route_stops.end(), std::next(route->stops.rbegin()), route->stops.rend());

            for (const auto& stop : route_stops) {
                polyline.AddPoint(sphere_projector(stop->coords));
            }

            polyline.SetStrokeColor(render_settings_.color_palette[color_num]);
            polyline.SetFillColor("none");
            polyline.SetStrokeWidth(render_settings_.line_width);
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            if (color_num < (render_settings_.color_palette.size() - 1)) {
                ++color_num;
            }
            else {
                color_num = 0;
            }

            polylines.push_back(polyline);
        }

        return polylines;
    }

    std::vector<svg::Text> MapRenderer::CreateRouteNames(const std::map<std::string_view, const transport::BusRoute*>& sorted_bus, const SphereProjector& sphere_projector) const {
        std::vector<svg::Text> route_names;
        size_t color_num = 0;

        for (const auto& [bus_number, route] : sorted_bus) {
            if (route->stops.empty()) {
                continue;
            }

            const auto& first_stop_coords = sphere_projector(route->stops[0]->coords);
            AddRouteName(route_names, route, first_stop_coords, color_num);

            if (route->is_circular == false && route->stops[0] != route->stops[route->stops.size() - 1]) {
                const auto& last_stop_coords = sphere_projector(route->stops[route->stops.size() - 1]->coords);
                AddRouteName(route_names, route, last_stop_coords, color_num);
            }

            color_num = (color_num + 1) % render_settings_.color_palette.size();
        }

        return route_names;
    }

    void MapRenderer::AddRouteName(std::vector<svg::Text>& route_names, const transport::BusRoute* route, const svg::Point& position, size_t& color_num) const {
        svg::Text text;
        svg::Text underlayer;

        text.SetPosition(position);
        text.SetOffset(render_settings_.bus_label_offset);
        text.SetFontSize(render_settings_.bus_label_font_size);
        text.SetFontFamily("Verdana");
        text.SetFontWeight("bold");
        text.SetData(route->name);
        text.SetFillColor(render_settings_.color_palette[color_num]);

        underlayer.SetPosition(position);
        underlayer.SetOffset(render_settings_.bus_label_offset);
        underlayer.SetFontSize(render_settings_.bus_label_font_size);
        underlayer.SetFontFamily("Verdana");
        underlayer.SetFontWeight("bold");
        underlayer.SetData(route->name);
        underlayer.SetFillColor(render_settings_.underlayer_color);
        underlayer.SetStrokeColor(render_settings_.underlayer_color);
        underlayer.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        route_names.push_back(underlayer);
        route_names.push_back(text);
    }

    std::vector<svg::Circle> MapRenderer::CreateStopSymbols(const std::map<std::string_view, const transport::Stop*>& sorted_stops, const SphereProjector& sphere_projector) const {
        std::vector<svg::Circle> symbols;
        for (const auto& [stop_name, stop] : sorted_stops) {
            svg::Circle symbol;
            symbol.SetCenter(sphere_projector(stop->coords));
            symbol.SetRadius(render_settings_.stop_radius);
            symbol.SetFillColor("white");

            symbols.push_back(symbol);
        }

        return symbols;
    }

    std::vector<svg::Text> MapRenderer::CreateStopsNames(const std::map<std::string_view, const transport::Stop*>& sorted_stops, const SphereProjector& sphere_projector) const {
        std::vector<svg::Text> stop_names;
        svg::Text text;
        svg::Text underlayer;
        for (const auto& [stop_name, stop] : sorted_stops) {
            text.SetPosition(sphere_projector(stop->coords));
            text.SetOffset(render_settings_.stop_label_offset);
            text.SetFontSize(render_settings_.stop_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetData(stop->name);
            text.SetFillColor("black");

            underlayer.SetPosition(sphere_projector(stop->coords));
            underlayer.SetOffset(render_settings_.stop_label_offset);
            underlayer.SetFontSize(render_settings_.stop_label_font_size);
            underlayer.SetFontFamily("Verdana");
            underlayer.SetData(stop->name);
            underlayer.SetFillColor(render_settings_.underlayer_color);
            underlayer.SetStrokeColor(render_settings_.underlayer_color);
            underlayer.SetStrokeWidth(render_settings_.underlayer_width);
            underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            stop_names.push_back(underlayer);
            stop_names.push_back(text);
        }

        return stop_names;
    }

} // namespace map