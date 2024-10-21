#include "json_reader.h"

#include <vector>
#include <sstream>

using namespace std;

namespace reader{
    
    string ParseColor(const json::Node& color_node) {
    using namespace std;
        
    if (color_node.IsString()) {
        return color_node.AsString();
    } else if (color_node.IsArray()) {
        const auto& color_array = color_node.AsArray();
        
        if (color_array.size() == 3) {
            return  "rgb("s + to_string(color_array[0].AsInt()) + ","s + to_string(color_array[1].AsInt()) + ","s + to_string(color_array[2].AsInt()) + ")"s;
            
        } else if (color_array.size() == 4) {
            std::stringstream out;
            
            out<<"rgba("s + to_string(color_array[0].AsInt()) + ","s + to_string(color_array[1].AsInt()) + ","s + to_string(color_array[2].AsInt()) +","s;
            out<< color_array[3].AsDouble();
            out<<")"s;
            
            return out.str();
        }
    }
    return "none"s;
}

std::vector<string> ParseColorPalette(const json::Array& color_palette_node) {
    std::vector<string> palette;
    
    for (const auto& color : color_palette_node) {
        palette.push_back(ParseColor(color));
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
    settings.underlayer_color = ParseColor(render_settings_dict.at("underlayer_color"s));
    settings.underlayer_width = render_settings_dict.at("underlayer_width"s).AsDouble();
    settings.color_palette = ParseColorPalette(render_settings_dict.at("color_palette"s).AsArray());
    return settings;
}

const std::string& GetTypeRequests(const json::Node& request){
    return request.AsMap().at("type"s).AsString();
}

const std::string& GetNameRequests(const json::Node& request){
    return request.AsMap().at("name"s).AsString();
}

int GetIdRequests(const json::Node& request){
    return request.AsMap().at("id"s).AsInt();
}

    JsonHandler::JsonHandler(istream& input, handler::RequestHandler& handler, map_renderer::MapRenderer& renderer) : 
    handler_(handler), renderer_(renderer){
        document_ = json::Load(input);
        renderer_(ParseRenderSettings(document_));
    }

    void JsonHandler::ProcessInput(){
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
        for (const auto& request : GetBaseRequests()){
            if (GetTypeRequests(request) == "Bus"s) {
                AddBus(request);
            }
        }
    }
    
    void JsonHandler::ProcessRenderMap(std::ostream& output){
        renderer_.RenderMap(handler_.GetAllStops(), handler_.GetAllBuses(), output);
    }

    void JsonHandler::ProcessOutput(std::ostream& output) {
        json::Array answers;
        
        for (const auto& request : GetStatRequests()){
            
            if(GetTypeRequests(request) == "Bus"s){
                GetInfoBus(request, answers);
            }else if(GetTypeRequests(request) == "Stop"s){
                GetInfoStop(request, answers);
            }else {
                RenderMapResponse(request, answers);
            }
        }
        
        json::Print(json::Document{answers}, output);
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
        
        for(const auto& [stop, dist] : request.AsMap().at("road_distances"s).AsMap()){
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

        if(!request.AsMap().at("is_roundtrip"s).AsBool()){
            auto copy_stops = stop_names;
            bool first = true;

            for (auto it = copy_stops.rbegin(); it != copy_stops.rend(); ++it){
                if(first){
                    first = false;
                    continue;
                }
                stop_names.push_back(*it);
            }
        }
        handler_.AddBus(name, stop_names, request.AsMap().at("is_roundtrip").AsBool());
    }

    void JsonHandler::GetInfoBus(const json::Node& request, json::Array& answers) {
        if(!handler_.GetBusStat(GetNameRequests(request))){
            
            json::Node no_bus{json::Dict{{"request_id"s, GetIdRequests(request)},{"error_message"s, "not found"s}}};
            answers.push_back(no_bus);
            
        }else{
            auto BusInfo = handler_.GetBusStat(GetNameRequests(request)).value();
            
            json::Node bus_info{json::Dict{{"curvature"s, BusInfo.curve},{"request_id"s, GetIdRequests(request)},{"route_length"s, BusInfo.route_length},{"stop_count"s, BusInfo.stop_count},{"unique_stop_count"s,BusInfo.unique_stop_count}}};
            
            answers.push_back(bus_info);
        }
    }

    void JsonHandler::GetInfoStop(const json::Node& request, json::Array& answers) {
        if(handler_.GetBusesByStop(GetNameRequests(request)).empty() && !handler_.YesOrNoStop(GetNameRequests(request))){
            
            json::Node no_stop{json::Dict{{"request_id"s, GetIdRequests(request)},{"error_message"s, "not found"s}}};
            answers.push_back(no_stop);
            
        }else{
        auto buses_to_stop = handler_.GetBusesByStop(GetNameRequests(request));
        json::Array buses;
            
        for(auto& bus_name : buses_to_stop){
            std::string s(bus_name);
            json::Node bus(s);
            buses.push_back(bus);
        }
            
        json::Node buses_n{json::Dict{{"buses"s, buses},{"request_id"s, GetIdRequests(request)}}};
        answers.push_back(buses_n);
        }
    }
    
    void JsonHandler::RenderMapResponse(const json::Node& request, json::Array& answers) {
        std::ostringstream svg_output;
        renderer_.RenderMap(handler_.GetAllStops(), handler_.GetAllBuses(), svg_output);

        std::string svg_str = svg_output.str();
    
        answers.push_back(json::Dict{
            {"request_id"s, GetIdRequests(request)},
            {"map"s, svg_str}
        });
    }

}