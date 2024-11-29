#pragma once

#include <memory>
#include <optional>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

class TransportRouter {
public:
    TransportRouter() = default;

    TransportRouter(const catalogue::TransportCatalogue& catalogue, RoutingSettings settings);
    
    TransportRouter(const TransportRouter&) = delete;
    TransportRouter& operator=(const TransportRouter&) = delete;
    
    TransportRouter(TransportRouter&& other) noexcept;
    
    TransportRouter& operator=(TransportRouter&& other) noexcept;

    const graph::Edge<double>& GetEdge (size_t id) const;

    const RoutingSettings& GetRoutingSettings() const;

    const std::optional<graph::Router<double>::RouteInfo> BuildRoute (const std::string& from, const std::string& to) const;

private:
    const catalogue::TransportCatalogue* catalogue_ = nullptr;
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    
    void BuildGraph();
};

}