#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include "geo.h"

namespace catalogue{

struct Stop{
    std::string name;
    geo::Coordinates coord;
};

struct Bus{
    std::string name;
    std::vector<Stop*> stops;
};

class TransportCatalogue {
	public:
    void AddStop(const Stop& stop_new);
    
    Stop* FindStop(std::string_view name_stop) const;
    
    void AddBus(const Bus& bus_new);
    
    Bus* FindBus(std::string_view name_bus) const;
    
    std::set<std::string_view> GetStopInfo(std::string_view name_stop) const;
    
    private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stop_quest_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> bus_quest_;
};
}