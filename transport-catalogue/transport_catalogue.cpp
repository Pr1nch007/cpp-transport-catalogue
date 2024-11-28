#include "transport_catalogue.h"

#include <algorithm>

using namespace std;

namespace catalogue {

void TransportCatalogue::AddStop(const Stop& stop_new) {
    stops_.push_back(move(stop_new));
    stop_quest_[stops_.back().name] = &stops_.back();
}

Stop* TransportCatalogue::FindStop(string_view name_stop) const {
    auto stop_iter = stop_quest_.find(name_stop);
    
    if(stop_iter == stop_quest_.end()){
        return nullptr;
    }
    return stop_iter->second;
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip) {
    domain::Bus bus_new;
    bus_new.name = move(name);
    
    for (const auto& stop_name : stops) {
        domain::Stop* stop = FindStop(stop_name);
        bus_new.stops.push_back(stop);
    }
    
    bus_new.is_roundtrip = is_roundtrip;
    buses_.push_back(std::move(bus_new));
    bus_quest_[buses_.back().name] = &buses_.back();
}

Bus* TransportCatalogue::FindBus(string_view name_bus) const {
    auto bus_iter = bus_quest_.find(name_bus);
    
    if(bus_iter == bus_quest_.end()) {
        return nullptr;
    }
    return bus_iter->second;
}

set<string_view> TransportCatalogue::GetStopInfo(string_view name_stop) const {
    auto stop = FindStop(name_stop);
    set<string_view> buses_to_stop;
    
    if(stop == nullptr) {
            return {};
    } else {
        for(auto& i : buses_){
            if(find(i.stops.begin(), i.stops.end(), stop) != i.stops.end()) {
                buses_to_stop.insert(i.name);
            }
        }
    }
    return buses_to_stop;
}
    
void TransportCatalogue::AddDistance(string_view from_stop, string_view to_stop, int distance) {
    distance_to_stops_[make_pair(FindStop(from_stop), FindStop(to_stop))] = distance;
}
    
int TransportCatalogue::FindDistance(string_view from_stop, string_view to_stop) const {
    auto stop1 = FindStop(from_stop);
    auto stop2 = FindStop(to_stop);
    auto dist = distance_to_stops_.find(make_pair(stop1, stop2));
    
    if(dist == distance_to_stops_.end()) {
        auto dist2 = distance_to_stops_.find(make_pair(stop2, stop1));
        
        if(dist2 == distance_to_stops_.end()) {
            return 0;
        }
        
        return dist2->second;
    }
    return dist->second;
}
    
std::optional<BusStat> TransportCatalogue::GetBusInfo(const std::string_view& bus_name) const {
    if(FindBus(bus_name) == nullptr) {
        return {};
    }
    domain::Bus bus = *FindBus(bus_name);
    int fact_dist = 0;
    double geo_dist = 0.0;
    std::set<std::string_view> unique_stop;
    
    for(size_t i = 0; i < bus.stops.size(); ++i) {
        unique_stop.insert(bus.stops[i]->name);

        if(i == 0) {
            continue;
        }

        geo_dist += geo::ComputeDistance(bus.stops[i-1]->coord, bus.stops[i]->coord);
        fact_dist += FindDistance(bus.stops[i-1]->name, bus.stops[i]->name);
    }
    double curv = static_cast<double>(fact_dist) / geo_dist;
    return std::optional<BusStat>{{curv, fact_dist, static_cast<int>(bus.stops.size()), static_cast<int>(unique_stop.size())}};
}
    
std::set<Stop*> TransportCatalogue::GetStopsInRoutes() const {
    std::set<Stop*> stops_in_routes;
    for (const auto& [name, bus] : bus_quest_) {
        if(bus->stops.empty()) continue;
        
        for (const auto& stop : bus->stops) {
            if(FindStop(stop->name) == nullptr) continue;
            stops_in_routes.insert(stop);
        }
    }
    return stops_in_routes;
}

    const std::map<std::string_view, Bus*>& TransportCatalogue::GetAllBuses() const{
        return bus_quest_;
    }
    
graph::DirectedWeightedGraph<double> TransportCatalogue::BuildGraph(int bus_wait_time, double bus_velocity) {
    graph::DirectedWeightedGraph<double> graph(stops_.size());

    size_t index = 0;
    for (const auto& stop : stops_) {
        stop_to_index_[stop.name] = index++;
    }

    for (const auto& [_, bus] : bus_quest_) {
        const auto& stops = bus->stops;
        for (size_t i = 0; i < stops.size(); ++i) {
            int stop_count = 0;
            double cumulative_distance = 0.0;
            for (size_t j = i + 1; j < stops.size(); ++j) {
                cumulative_distance += FindDistance(stops[j - 1]->name, stops[j]->name);
                double travel_time = cumulative_distance / (bus_velocity * 1000 / 60);
                ++stop_count;
                graph.AddEdge({
                    stop_to_index_.at(stops[i]->name),
                    stop_to_index_.at(stops[j]->name),
                    bus_wait_time + travel_time,
                    _,
                    stop_count
                });
            }
        }
    }

    return graph;
}
    
size_t TransportCatalogue::FindStopIndex(const std::string& stop_name) const {
    auto it = stop_to_index_.find(stop_name);
    if (it != stop_to_index_.end()) {
        return it->second;
    }
    throw std::out_of_range("Stop not found in stop_to_index"s);
}
    
std::string_view TransportCatalogue::GetStopToIndex (size_t id) const {
    return stops_[id].name;
} 
    
}