#include "transport_router.h"

namespace router{

    TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue, RoutingSettings settings)
        : routing_settings_(std::move(settings)), graph_(BuildGraph(catalogue)) {
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }

    const RoutingSettings& TransportRouter::GetRoutingSettings() const {
        return routing_settings_;
        
    }

    const std::optional<RouteInfo> TransportRouter::BuildRoute (const std::string& from, const std::string& to, const catalogue::TransportCatalogue& catalogue) const {
        if (!router_) {
            throw std::logic_error("Router is not initialized");
        }

        auto route = router_->BuildRoute(catalogue.FindStopIndex(from), catalogue.FindStopIndex(to));
        if (!route) {
            return std::nullopt;
        }

        std::vector<graph::Edge<double>> edges;
        for (const auto& edge_id : route->edges) {
            edges.push_back(graph_.GetEdge(edge_id));
        }
        return RouteInfo{route->weight, std::move(edges)};
    }

    graph::DirectedWeightedGraph<double> TransportRouter::BuildGraph(const catalogue::TransportCatalogue& catalogue) const {
    graph::DirectedWeightedGraph<double> graph(catalogue.GetAllStops().size());
        for (const auto& [_, bus] : catalogue.GetAllBuses()) {
            const auto& stops = bus->stops;
            for (size_t i = 0; i < stops.size(); ++i) {
                int stop_count = 0;
                double cumulative_distance = 0.0;
                for (size_t j = i + 1; j < stops.size(); ++j) {
                    cumulative_distance += catalogue.FindDistance(stops[j - 1]->name, stops[j]->name);
                    double travel_time = cumulative_distance / (routing_settings_.bus_velocity * KM_TO_METERS / MINUTES_IN_HOUR);
                    ++stop_count;
                    graph.AddEdge({
                        catalogue.FindStopIndex(stops[i]->name),
                        catalogue.FindStopIndex(stops[j]->name),
                        routing_settings_.bus_wait_time + travel_time,
                        bus->name,
                        stop_count
                    });
                }
            }
        }
        return graph;
    }
}