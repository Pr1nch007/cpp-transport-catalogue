#pragma once

#include <memory>
#include <optional>

#include "graph.h"
#include "router.h"

namespace router {

struct RoutingSettings {
    int bus_wait_time;
    double bus_velocity;
};

class TransportRouter {
public:
    TransportRouter() = default;

    void Initialize(graph::DirectedWeightedGraph<double> graph, RoutingSettings settings);

    const graph::DirectedWeightedGraph<double>& GetGraph() const;

    const graph::Edge<double>& GetEdge (size_t id) const;
    
    const graph::Router<double>& GetRouter() const;
      

    const RoutingSettings& GetRoutingSettings() const;

    const std::optional<graph::Router<double>::RouteInfo> BuildRoute (size_t from, size_t to) const;

private:
    graph::DirectedWeightedGraph<double> graph_;
    RoutingSettings routing_settings_;
    std::unique_ptr<graph::Router<double>> router_;
};

}