#pragma once

#include <iostream>
#include <string>
#include <variant>

#include "json.h"
#include "json_builder.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace reader{
    
struct Rgb {
    int red;
    int green;
    int blue;
};

struct Rgba {
    int red;
    int green;
    int blue;
    double alpha;
};
    
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    
struct ColorInString {
    std::string operator()(std::monostate);

    std::string operator()(std:: string color);

    std::string operator()(Rgb color);

    std::string operator()(Rgba color);
};
    
Color ParseColor(const json::Node& color_node);

std::vector<std::string> ParseColorPalette(const json::Array& color_palette_node);

map_renderer::RenderSettings ParseRenderSettings(const json::Document& json_doc);

const std::string& GetTypeRequests(const json::Node& request);

const std::string& GetNameRequests(const json::Node& request);

int GetIdRequests(const json::Node& request);

class JsonHandler {
public:
    JsonHandler(std::istream& input, 
                handler::RequestHandler& handler, 
                map_renderer::MapRenderer& renderer);
    
    void ProcessInput();
    
    void ProcessRenderMap(std::ostream& output);

    void ProcessOutput(std::ostream& output);

private:
    handler::RequestHandler& handler_;
    map_renderer::MapRenderer& renderer_;
    json::Document document_;
    std::unique_ptr<router::TransportRouter> router_;
    
    const json::Array& GetBaseRequests() const;
    const json::Array& GetStatRequests() const;
    const json::Dict& GetRoutingSettings() const;
    
    void AddStop (const json::Node& request);
    void AddDistance(const json::Node& request);
    void AddBus(const json::Node& request);
    
    void GetInfoBus(const json::Node& request, json::Builder& builder);
    void GetInfoStop(const json::Node& request, json::Builder& builder);
    
    void RenderMapResponse(const json::Node& request, json::Builder& builder);
    
    router::RoutingSettings ProcessRoutingSettings(const json::Dict& routing_settings) const;
    
    void ProcessRouteRequest(const json::Node& request, json::Builder& builder);
};
    
}