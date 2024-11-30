#pragma once

#include <memory>
#include <optional>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace router {

static constexpr double KM_TO_METERS = 1000.0;
static constexpr double MINUTES_IN_HOUR = 60.0;    
    
struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};
    
struct RouteInfo {
    double full_time = 0.0;
    std::vector<graph::Edge<double>> edges;
};    

class TransportRouter {
public:
    TransportRouter() = default;

    TransportRouter(const catalogue::TransportCatalogue& catalogue, RoutingSettings settings);

    const RoutingSettings& GetRoutingSettings() const;

    const std::optional<RouteInfo> BuildRoute (const std::string& from, const std::string& to, const catalogue::TransportCatalogue& catalogue) const ;

private:
    RoutingSettings routing_settings_;
    graph::DirectedWeightedGraph<double> graph_;
    std::unique_ptr<graph::Router<double>> router_;

    graph::DirectedWeightedGraph<double> BuildGraph(const catalogue::TransportCatalogue& catalogue) const;
};

}