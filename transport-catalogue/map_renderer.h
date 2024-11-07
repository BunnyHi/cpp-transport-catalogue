#pragma once

#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <map>
#include <algorithm>
#include <set>
#include <string>

namespace map {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding)
        {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.longitude < rhs.longitude; });
            min_lon_ = left_it->longitude;
            const double max_lon = right_it->longitude;

            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.latitude < rhs.latitude; });
            const double min_lat = bottom_it->latitude;
            max_lat_ = top_it->latitude;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.longitude - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.latitude) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct RenderSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width = 0.0;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset = { 0.0, 0.0 };
        int stop_label_font_size = 0;
        svg::Point stop_label_offset = { 0.0, 0.0 };
        svg::Color underlayer_color = { svg::NoneColor };
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette{};
    };

    class MapRenderer {
    public:
        MapRenderer(const RenderSettings& render_settings,
            const transport::TransportCatalogue& catalogue)
            : render_settings_(render_settings),
            catalogue_(catalogue)
            {}

        svg::Document RenderMap() const;

    private:

        const RenderSettings render_settings_;
        const transport::TransportCatalogue& catalogue_;
        

        svg::Document CreateSVGDocument(const std::map<std::string_view, const transport::BusRoute*>& sorted_bus, const std::map<std::string_view, const transport::Stop*> sorted_stops) const;

        std::vector<svg::Polyline> CreatePolylines(const std::map<std::string_view, const transport::BusRoute*>& sorted_bus, const SphereProjector& sphere_projector) const;

        std::vector<svg::Text> CreateRouteNames(const std::map<std::string_view, const transport::BusRoute*>& sorted_bus, const SphereProjector& sphere_projector) const;
        void AddRouteName(std::vector<svg::Text>& route_names, const transport::BusRoute* route, const svg::Point& position, size_t& color_num) const;

        std::vector<svg::Circle> CreateStopSymbols(const std::map<std::string_view, const transport::Stop*>& sorted_stops, const SphereProjector& sphere_projector) const;

        std::vector<svg::Text> CreateStopsNames(const std::map<std::string_view, const transport::Stop*>& sorted_stops, const SphereProjector& sphere_projector) const;

    };

} // namespace map