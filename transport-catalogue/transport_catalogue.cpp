#include "transport_catalogue.h"

#include <algorithm>

using namespace std;

namespace catalogue{

void TransportCatalogue::AddStop(const Stop& stop_new){
    stops_.push_back(move(stop_new));
    stop_quest_[stops_.back().name] = &stops_.back();
}

Stop* TransportCatalogue::FindStop(string_view name_stop) const{
    auto stop_iter = stop_quest_.find(name_stop);
    
    if(stop_iter == stop_quest_.end()){
        return nullptr;
    }
    
    return stop_iter->second;
}

void TransportCatalogue::AddBus(const Bus& bus_new){
    buses_.push_back(move(bus_new));
    bus_quest_[buses_.back().name] = &buses_.back();
}

Bus* TransportCatalogue::FindBus(string_view name_bus) const{
    auto bus_iter = bus_quest_.find(name_bus);
    
    if(bus_iter == bus_quest_.end()){
        return nullptr;
    }
    
    return bus_iter->second;
}

set<string_view> TransportCatalogue::GetStopInfo(string_view name_stop) const{
    auto stop = FindStop(name_stop);
        set<string_view> buses_to_stop;
    
        if(stop == nullptr){
            return {};
        }else{
            for(auto& i : buses_){
                
                if(find(i.stops.begin(), i.stops.end(), stop) != i.stops.end()){
                    buses_to_stop.insert(i.name);
                }
            }
        }
        return buses_to_stop;
}
    
void TransportCatalogue::AddDistance(string_view from_stop, string_view to_stop, int distance){
    distance_to_stops_[make_pair(FindStop(from_stop), FindStop(to_stop))] = distance;
}
    
int TransportCatalogue::FindDistance(string_view from_stop, string_view to_stop) const{
    auto stop1 = FindStop(from_stop);
    auto stop2 = FindStop(to_stop);
    auto dist = distance_to_stops_.find(make_pair(stop1, stop2));
    
    if(dist == distance_to_stops_.end()){
        auto dist2 = distance_to_stops_.find(make_pair(stop2, stop1));
        
        if(dist2 == distance_to_stops_.end()){
            return 0;
        }
        
        return dist2->second;
    }
    
    return dist->second;
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
    
}