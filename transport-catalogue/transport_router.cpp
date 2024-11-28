#include "transport_router.h"

namespace router{

    void TransportRouter::Initialize(graph::DirectedWeightedGraph<double> graph, RoutingSettings settings) {
        graph_ = std::move(graph);
        routing_settings_ = std::move(settings);
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
        return graph_;
    }

    const graph::Edge<double>& TransportRouter::GetEdge (size_t id) const {
        return graph_.GetEdge(id);
    }

const graph::Router<double>& TransportRouter::GetRouter() const {
        return *router_;
    }

    const RoutingSettings& TransportRouter::GetRoutingSettings() const {
        return routing_settings_;
        
    }

    const std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute (size_t from, size_t to) const {
        return router_->BuildRoute(from, to);
    }

}