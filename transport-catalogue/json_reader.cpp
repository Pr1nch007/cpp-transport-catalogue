#include "json_reader.h"

#include <vector>
#include <sstream>

using namespace std;

namespace reader {
    
std::string ColorInString::operator()(std::monostate) {
    return "none"s;
}

std::string ColorInString::operator()(std:: string color) {
    return color;
}

std::string ColorInString::operator()(Rgb color) {
    return "rgb("s + std::to_string(color.red) + ","s + std::to_string(color.green) + ","s + std::to_string(color.blue) + ")"s;
}

std::string ColorInString::operator()(Rgba color) {
    std::stringstream out;
    
    out << "rgba("s + to_string(color.red) + ","s + to_string(color.green) + ","s + to_string(color.blue) +","s;
    
    out<< color.alpha;
    out<<")"s;
    return out.str();
}

    
Color ParseColor(const json::Node& color_node) {
    using namespace std;
    if (color_node.IsString()) {
        return color_node.AsString();
    } else if (color_node.IsArray()) {
        const auto& color_array = color_node.AsArray();
        if (color_array.size() == string("bus"s).size()) {  
            return Rgb{
                color_array[0].AsInt(),
                color_array[1].AsInt(),
                color_array[2].AsInt()
            };
        } else if (color_array.size() == string("stop"s).size()) {  
            return Rgba{
                color_array[0].AsInt(),
                color_array[1].AsInt(),
                color_array[2].AsInt(),
                color_array[3].AsDouble()
            };
        }
    }
    return std::monostate{};
}
    
std::vector<string> ParseColorPalette(const json::Array& color_palette_node) {
    std::vector<string> palette;
    
    for (const auto& color : color_palette_node) {
        palette.push_back(std::visit(ColorInString{} ,ParseColor(color)));
    }
    
    return palette;   
}

map_renderer::RenderSettings ParseRenderSettings(const json::Document& json_doc) {
    map_renderer::RenderSettings settings;
    const auto& render_settings_dict = json_doc.GetRoot().AsMap().at("render_settings"s).AsMap();
    
    settings.width = render_settings_dict.at("width"s).AsDouble();
    settings.height = render_settings_dict.at("height"s).AsDouble();
    settings.padding = render_settings_dict.at("padding"s).AsDouble();
    settings.line_width = render_settings_dict.at("line_width"s).AsDouble();
    settings.stop_radius = render_settings_dict.at("stop_radius"s).AsDouble();
    settings.bus_label_font_size = render_settings_dict.at("bus_label_font_size"s).AsInt();
    settings.bus_label_offset = {render_settings_dict.at("bus_label_offset"s).AsArray().at(0).AsDouble(),
                                 render_settings_dict.at("bus_label_offset"s).AsArray().at(1).AsDouble()};
    settings.stop_label_font_size = render_settings_dict.at("stop_label_font_size"s).AsInt();
    settings.stop_label_offset = {render_settings_dict.at("stop_label_offset"s).AsArray().at(0).AsDouble(),
                                  render_settings_dict.at("stop_label_offset"s).AsArray().at(1).AsDouble()};
    settings.underlayer_color = std::visit(ColorInString{} ,ParseColor(render_settings_dict.at("underlayer_color"s)));
    settings.underlayer_width = render_settings_dict.at("underlayer_width"s).AsDouble();
    settings.color_palette = ParseColorPalette(render_settings_dict.at("color_palette"s).AsArray());
    return settings;
}

const std::string& GetTypeRequests(const json::Node& request) {
    return request.AsMap().at("type"s).AsString();
}

const std::string& GetNameRequests(const json::Node& request) {
    return request.AsMap().at("name"s).AsString();
}

int GetIdRequests(const json::Node& request) {
    return request.AsMap().at("id"s).AsInt();
}

JsonHandler::JsonHandler(istream& input, handler::RequestHandler& handler, map_renderer::MapRenderer& renderer, router::TransportRouter& router) : 
handler_(handler), renderer_(renderer), router_(router) {
    document_ = json::Load(input);
    renderer_(ParseRenderSettings(document_));
}

void JsonHandler::ProcessInput(){
    const auto& root_map = document_.GetRoot().AsMap();
    
    for (const auto& request : GetBaseRequests()) {
        if (GetTypeRequests(request) == "Stop"s) {
            AddStop(request);
        }
    }
    for (const auto& request : GetBaseRequests()) {
        if (GetTypeRequests(request) == "Stop"s) {
            AddDistance(request);
        }
    }
    for (const auto& request : GetBaseRequests()) {
        if (GetTypeRequests(request) == "Bus"s) {
            AddBus(request);
        }
    }
    router::RoutingSettings routing_settings = ProcessRoutingSettings(root_map.at("routing_settings"s));
    router_.Initialize(handler_.BuildGraph(routing_settings.bus_wait_time, routing_settings.bus_velocity), std::move(routing_settings));
}

    
void JsonHandler::ProcessRenderMap(std::ostream& output) {
    renderer_.RenderMap(handler_.GetAllStops(), handler_.GetAllBuses(), output);
}

void JsonHandler::ProcessOutput(std::ostream& output) {
    json::Builder builder;
    builder.StartArray();
        
    for (const auto& request : GetStatRequests()) {           
        if(GetTypeRequests(request) == "Bus"s) {
            GetInfoBus(request, builder);
        } else if(GetTypeRequests(request) == "Stop"s) {
            GetInfoStop(request, builder);
        } else if(GetTypeRequests(request) == "Route"s){
            ProcessRouteRequest(request, builder);
        } else {
            RenderMapResponse(request, builder);
        }
    }

    builder.EndArray();
    json::Print(json::Document{builder.Build()}, output);
}
    
const json::Array& JsonHandler::GetBaseRequests() const {
    return document_.GetRoot().AsMap().at("base_requests"s).AsArray();
}

const json::Array& JsonHandler::GetStatRequests() const {
    return document_.GetRoot().AsMap().at("stat_requests"s).AsArray();
}
    
void JsonHandler::AddStop (const json::Node& request) {
    std::string name = GetNameRequests(request);
        
    double lat = request.AsMap().at("latitude"s).AsDouble();
    double lng = request.AsMap().at("longitude"s).AsDouble();
        
    handler_.AddStop(name, lat, lng);
}
    
void JsonHandler::AddDistance(const json::Node& request) { 
    for(const auto& [stop, dist] : request.AsMap().at("road_distances"s).AsMap()) {
        handler_.AddDistance(GetNameRequests(request), stop, dist.AsInt());
    }
}

void JsonHandler::AddBus(const json::Node& request) {
    std::string name = GetNameRequests(request);
    const auto& stops = request.AsMap().at("stops"s).AsArray();
    std::vector<std::string> stop_names;

    for (const auto& stop : stops) {
        stop_names.push_back(stop.AsString());
    }

    if(!request.AsMap().at("is_roundtrip"s).AsBool()) {
        auto copy_stops = stop_names;
        bool first = true;

        for (auto it = copy_stops.rbegin(); it != copy_stops.rend(); ++it) {
            if(first) {
                first = false;
                continue;
            }
            stop_names.push_back(*it);
        }
    }
    handler_.AddBus(name, stop_names, request.AsMap().at("is_roundtrip").AsBool());
}

void JsonHandler::GetInfoBus(const json::Node& request, json::Builder& builder) {
    auto bus_stat = handler_.GetBusStat(GetNameRequests(request));

    if(!bus_stat) {        
        builder.StartDict()
               .Key("request_id"s).Value(GetIdRequests(request))
               .Key("error_message"s).Value("not found"s)
               .EndDict();  
    } else {
        auto& info = bus_stat.value();

        builder.StartDict()
               .Key("curvature"s).Value(info.curve)
               .Key("request_id"s).Value(GetIdRequests(request))
               .Key("route_length"s).Value(info.route_length)
               .Key("stop_count"s).Value(info.stop_count)
               .Key("unique_stop_count"s).Value(info.unique_stop_count)
               .EndDict();
    }
}

void JsonHandler::GetInfoStop(const json::Node& request, json::Builder& builder) {
    auto buses_to_stop = handler_.GetBusesByStop(GetNameRequests(request));
    
    if(buses_to_stop.empty() && !handler_.CheckStop(GetNameRequests(request))) {
        builder.StartDict()
               .Key("request_id"s).Value(GetIdRequests(request))
               .Key("error_message"s).Value("not found"s)
               .EndDict();
    } else {
        builder.StartDict()
               .Key("request_id"s).Value(GetIdRequests(request))
               .Key("buses"s).StartArray();
        
        for (const auto& bus_name : buses_to_stop) {
            builder.Value(std::string(bus_name));
        }
        
        builder.EndArray().EndDict();
    }
}
    
void JsonHandler::RenderMapResponse(const json::Node& request, json::Builder& builder) {
    std::ostringstream svg_output;
    renderer_.RenderMap(handler_.GetAllStops(), handler_.GetAllBuses(), svg_output);
    std::string svg_str = svg_output.str();
        
    builder.StartDict()
           .Key("request_id"s).Value(GetIdRequests(request))
           .Key("map"s).Value(svg_str)
           .EndDict();
}

    router::RoutingSettings JsonHandler::ProcessRoutingSettings(const json::Node& routing_settings) const {
        const auto& settings_map = routing_settings.AsMap();
        return {
        settings_map.at("bus_wait_time"s).AsInt(),
        settings_map.at("bus_velocity"s).AsDouble()
        };
    }
    
    void JsonHandler::ProcessRouteRequest(const json::Node& request, json::Builder& builder) {
    const auto& from = request.AsMap().at("from"s).AsString();
    const auto& to = request.AsMap().at("to"s).AsString();

    if (!handler_.CheckStop(from) || !handler_.CheckStop(to)) {
        builder.StartDict()
            .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
            .Key("error_message"s).Value("not found"s)
            .EndDict();
        return;
    }

    auto route = router_.BuildRoute(handler_.FindStopIndex(from), handler_.FindStopIndex(to));
    if (!route) {
        builder.StartDict()
            .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
            .Key("error_message"s).Value("not found"s)
            .EndDict();
        return;
    }
        
    const auto& [full_time, edges] = *route;
    builder.StartDict()
        .Key("request_id"s).Value(request.AsMap().at("id"s).AsInt())
        .Key("total_time"s).Value(full_time)
        .Key("items"s).StartArray();

    for (const auto& edge_id : edges) {
        const auto& [index_from, 
                     index_to, 
                     time, 
                     bus_name, 
                     stop_count] = router_.GetEdge(edge_id);

            builder.StartDict()
            .Key("stop_name"s).Value(static_cast<std::string>(handler_.GetStopToIndex(index_from)))
            .Key("time"s).Value(router_.GetRoutingSettings().bus_wait_time)
            .Key("type"s).Value("Wait"s)
            .EndDict();

        double travel_time = time - router_.GetRoutingSettings().bus_wait_time;
        builder.StartDict()
            .Key("type"s).Value("Bus"s)
            .Key("span_count"s).Value(stop_count)
            .Key("time"s).Value(travel_time)
            .Key("bus"s).Value(static_cast<std::string>(bus_name))
            .EndDict();
    }

    builder.EndArray()
        .EndDict();
}
    
}