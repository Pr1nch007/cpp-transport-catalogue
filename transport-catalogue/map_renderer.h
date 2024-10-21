#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "svg.h"
#include "geo.h"
#include "domain.h"

namespace map_renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);
    
struct RenderSettings {
    double width;
    double height;
    double padding;
    double line_width;
    double stop_radius;
    uint32_t bus_label_font_size;
    svg::Point bus_label_offset;
    uint32_t stop_label_font_size;
    svg::Point stop_label_offset;
    svg::Color underlayer_color;
    double underlayer_width;
    std::vector<svg::Color> color_palette;
};    

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
    points_begin, points_end,
    [](const auto& lhs, const auto& rhs) { return lhs->coord.lng < rhs->coord.lng; });
min_lon_ = (*left_it)->coord.lng;
const double max_lon = (*right_it)->coord.lng;

const auto [bottom_it, top_it] = std::minmax_element(
    points_begin, points_end,
    [](const auto& lhs, const auto& rhs) { return lhs->coord.lat < rhs->coord.lat; });
const double min_lat = (*bottom_it)->coord.lat;
max_lat_ = (*top_it)->coord.lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer {
public:
    void operator()(RenderSettings settings);

    template <typename StopsRange, typename BusesRange>
    void RenderMap(const StopsRange& stops, const BusesRange& buses, std::ostream& output) {
        RenderBusLines(buses, stops);
        RenderBusLabels(buses, stops);
        RenderStopCircles(stops);
        RenderStopLabels(stops);
        doc_.Render(output);
    }

private:
    RenderSettings settings_;
    svg::Document doc_;
    
    // Рендеринг линий маршрутов
    template <typename BusesRange, typename StopsRange>
    void RenderBusLines(const BusesRange& buses, const StopsRange& stops) {
        using namespace std;
        
        SphereProjector projector(stops.begin(), stops.end(), settings_.width, settings_.height, settings_.padding);
        size_t color_idx = 0;

        for (const auto& [name, bus] : buses) {
            if (bus->stops.size() <= 1) continue;

            svg::Polyline polyline;
            polyline.SetStrokeColor(settings_.color_palette[color_idx % settings_.color_palette.size()])
                    .SetStrokeWidth(settings_.line_width)
                    .SetFillColor("none"s)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            
            for (const auto& stop : bus->stops) {
                polyline.AddPoint(projector(stop->coord));
            }

            doc_.Add(polyline);
            ++color_idx;
        }
    }
    
    void RenderBusLabel(svg::Point stop_point, const std::string& bus_name, const svg::Color& color);
    
    template <typename BusesRange, typename StopsRange>
    void RenderBusLabels(const BusesRange& buses, const StopsRange& stops)  {
        SphereProjector projector(stops.begin(), stops.end(), settings_.width, settings_.height, settings_.padding);
        size_t color_idx = 0;

        for (const auto& [bus_name, bus] : buses) {
            if (bus->stops.empty()) continue;

            auto bus_back = bus->stops[bus->stops.size()/2];
        
            svg::Point first_stop = projector(bus->stops.front()->coord);
            svg::Point last_stop = projector(bus_back->coord);
            svg::Color bus_color = settings_.color_palette[color_idx % settings_.color_palette.size()];
        
            RenderBusLabel(first_stop, bus_name.data(), bus_color);

            if (!bus->is_roundtrip && bus->stops.front() != bus_back) {
                RenderBusLabel(last_stop, bus_name.data(), bus_color);
            }
        
            ++color_idx;
        }
    }
    
    template <typename StopsRange>
    void RenderStopCircles(const StopsRange& stops) {
        SphereProjector projector(stops.begin(), stops.end(), settings_.width, settings_.height, settings_.padding);
    
      std::vector<domain::Stop*> sorted_stops(stops.begin(), stops.end());
        std::sort(sorted_stops.begin(), sorted_stops.end(), [](const auto* lhs, const auto* rhs) {
            return lhs->name < rhs->name;
        });

        for (const auto* stop : sorted_stops) {
            using namespace std;
            
            svg::Circle circle;
            circle.SetCenter(projector(stop->coord))
              .SetRadius(settings_.stop_radius)
              .SetFillColor("white"s);
            doc_.Add(circle);
        }
    }
    
    template <typename StopsRange>
    void RenderStopLabels(const StopsRange& stops) {
        SphereProjector projector(stops.begin(), stops.end(), settings_.width, settings_.height, settings_.padding);
    
        std::vector<domain::Stop*> sorted_stops(stops.begin(), stops.end());
        std::sort(sorted_stops.begin(), sorted_stops.end(), [](const auto* lhs, const auto* rhs) {
            return lhs->name < rhs->name;
        });

        for (const auto* stop : sorted_stops) {
            using namespace std;
            svg::Point stop_point = projector(stop->coord);
        
            svg::Text underlayer;
            underlayer.SetPosition(stop_point)
                  .SetOffset(settings_.stop_label_offset)
                  .SetFontSize(settings_.stop_label_font_size)
                  .SetFontFamily("Verdana"s)
                  .SetData(stop->name);
        
            svg::Text label = underlayer;
            label.SetFillColor("black"s);

            underlayer.SetFillColor(settings_.underlayer_color)
                  .SetStrokeColor(settings_.underlayer_color)
                  .SetStrokeWidth(settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
            doc_.Add(underlayer);
            doc_.Add(label);
        }
    }
};
    
}