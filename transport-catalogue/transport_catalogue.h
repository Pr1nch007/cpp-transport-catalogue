#pragma once

#include <deque>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>

#include "geo.h"
#include "domain.h"

namespace catalogue {

using namespace domain;
    
template<typename T>
class HashPairPoint {
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
    
    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);
    
    Bus* FindBus(std::string_view name_bus) const;
    
    std::set<std::string_view> GetStopInfo(std::string_view name_stop) const;
    
    void AddDistance(std::string_view from_stop, std::string_view to_stop, int distance);
    
    int FindDistance(std::string_view from_stop, std::string_view to_stop)const;
    
    std::optional<BusStat> GetBusInfo(const std::string_view& bus_name) const;
    
    std::set<Stop*> GetStopsInRoutes() const;

    const std::map<std::string_view, Bus*>& GetAllBuses() const;
    
private:
    std::deque<Stop> stops_;
    std::unordered_map<std::string_view, Stop*> stop_quest_;
    std::deque<Bus> buses_;
    std::map<std::string_view, Bus*> bus_quest_;
    std::unordered_map<std::pair<Stop*, Stop*>, int, HashPairPoint<Stop>> distance_to_stops_;
};
    
}