#include "transport_router.h"

namespace router{

    TransportRouter::TransportRouter(const catalogue::TransportCatalogue& catalogue, RoutingSettings settings)
        : catalogue_(&catalogue), routing_settings_(std::move(settings)) {
        BuildGraph();
    }

    TransportRouter::TransportRouter(TransportRouter&& other) noexcept
        : catalogue_(other.catalogue_),
          routing_settings_(std::move(other.routing_settings_)),
          graph_(std::move(other.graph_)) {
        router_ = std::make_unique<graph::Router<double>>(graph_);
      }
    
    TransportRouter& TransportRouter::operator=(TransportRouter&& other) noexcept {
        if (this != &other) {
            catalogue_ = other.catalogue_;
            routing_settings_ = std::move(other.routing_settings_);
            graph_ = std::move(other.graph_);
            router_ = std::make_unique<graph::Router<double>>(graph_);
        }
        return *this;
    }

    const graph::Edge<double>& TransportRouter::GetEdge (size_t id) const {
        return graph_.GetEdge(id);
    }

    const RoutingSettings& TransportRouter::GetRoutingSettings() const {
        return routing_settings_;
        
    }

    const std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute (const std::string& from, const std::string& to) const {
        return router_->BuildRoute(catalogue_->FindStopIndex(from), catalogue_->FindStopIndex(to));
    }

    void TransportRouter::BuildGraph() {
        graph_ = graph::DirectedWeightedGraph<double>(catalogue_->GetAllStops().size());
        for (const auto& [_, bus] : catalogue_->GetAllBuses()) {
            const auto& stops = bus->stops;
            for (size_t i = 0; i < stops.size(); ++i) {
                int stop_count = 0;
                double cumulative_distance = 0.0;
                for (size_t j = i + 1; j < stops.size(); ++j) {
                    cumulative_distance += catalogue_->FindDistance(stops[j - 1]->name, stops[j]->name);
                    double travel_time = cumulative_distance / (routing_settings_.bus_velocity * 1000 / 60);
                    ++stop_count;
                    graph_.AddEdge({
                        catalogue_->FindStopIndex(stops[i]->name),
                        catalogue_->FindStopIndex(stops[j]->name),
                        routing_settings_.bus_wait_time + travel_time,
                        bus->name,
                        stop_count
                    });
                }
            }
        }
        router_ = std::make_unique<graph::Router<double>>(graph_);
    }
}