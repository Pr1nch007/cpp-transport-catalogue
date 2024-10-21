#pragma once

#include <iostream>
#include <string>

#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

namespace reader{
    
std::string ParseColor(const json::Node& color_node);

std::vector<std::string> ParseColorPalette(const json::Array& color_palette_node);

map_renderer::RenderSettings ParseRenderSettings(const json::Document& json_doc);

const std::string& GetTypeRequests(const json::Node& request);

const std::string& GetNameRequests(const json::Node& request);

int GetIdRequests(const json::Node& request);

class JsonHandler {
public:
    JsonHandler(std::istream& input, handler::RequestHandler& handler, map_renderer::MapRenderer& renderer);

    void ProcessInput();
    
    void ProcessRenderMap(std::ostream& output);

    void ProcessOutput(std::ostream& output);

private:
    handler::RequestHandler& handler_;
    map_renderer::MapRenderer& renderer_;
    json::Document document_;
    
    const json::Array& GetBaseRequests() const;
    const json::Array& GetStatRequests() const;
    
    void AddStop (const json::Node& request);
    void AddDistance(const json::Node& request);
    void AddBus(const json::Node& request);
    
    void GetInfoBus(const json::Node& request, json::Array& answers);
    void GetInfoStop(const json::Node& request, json::Array& answers);
    
    void RenderMapResponse(const json::Node& request, json::Array& answers);
};

}