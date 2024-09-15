#include "transport_catalogue.h"

#include <algorithm>

using namespace std;

namespace catalogue{

void TransportCatalogue::AddStop(Stop stop_new){
    stops_.push_back(move(stop_new));
    stop_quest_[stops_.back().name] = &stops_.back();
}

Stop* TransportCatalogue::FindStop(string_view name_stop) const{
    if(!stop_quest_.count(name_stop)){
        return nullptr;
    }

    return stop_quest_.at(name_stop);
}

void TransportCatalogue::AddBus(Bus bus_new){
    buses_.push_back(move(bus_new));
    bus_quest_[buses_.back().name] = &buses_.back();
}

Bus* TransportCatalogue::FindBus(string_view name_bus) const{
    if(!bus_quest_.count(name_bus)){
        return nullptr;
    }

    return bus_quest_.at(name_bus);
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
}