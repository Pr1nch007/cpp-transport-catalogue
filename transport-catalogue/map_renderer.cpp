#include "map_renderer.h"

using namespace std;

namespace map_renderer{
    
    bool IsZero(double value) {
    return std::abs(value) < EPSILON;
    }
    
    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }
    
    void MapRenderer::operator()(RenderSettings settings) {
        settings_ = std::move(settings);
    }
    
    void MapRenderer::RenderBusLabel(svg::Point stop_point, const std::string& bus_name, const svg::Color& color) {
    svg::Text underlayer;
    underlayer.SetPosition(stop_point)
              .SetOffset(settings_.bus_label_offset)
              .SetFontSize(settings_.bus_label_font_size)
              .SetFontFamily("Verdana"s)
              .SetFontWeight("bold"s)
              .SetData(bus_name);

    svg::Text label = underlayer;
    label.SetFillColor(color);

    underlayer.SetFillColor(settings_.underlayer_color)
              .SetStrokeColor(settings_.underlayer_color)
              .SetStrokeWidth(settings_.underlayer_width)
              .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
              .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    doc_.Add(underlayer);
    doc_.Add(label);
    }
    
}