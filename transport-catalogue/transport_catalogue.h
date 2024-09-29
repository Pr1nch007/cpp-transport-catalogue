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
    
template<typename T>
class HashPairPoint{
    public:
    size_t operator()(const std::pair<T*, T*>& points) const {
    return hasher_(points.first)+ hasher_(points.second) * 37;
}
private:
    std::hash<const T*> hasher_;
};

class TransportCatalogue {
	public:
    void AddStop(const Stop& stop_new);
    
    Stop* FindStop(std::string_view name_stop) const;
    
    void AddBus(const Bus& bus_new);
    
    Bus* FindBus(std::string_view name_bus) const;
    
    std::set<std::string_view> GetStopInfo(std::string_view name_stop) const;
    
    void AddDistance(std::string_view from_stop, std::string_view to_stop, int distance);
    
    int FindDistance(std::string_view from_stop, std::string_view to_stop)const;
    
    private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stop_quest_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Bus*> bus_quest_;
    std::unordered_map<std::pair<Stop*, Stop*>, int, HashPairPoint<Stop>> distance_to_stops_;
};
}