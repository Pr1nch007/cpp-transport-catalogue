#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "transport_catalogue.h"
#include "domain.h"

namespace handler{

struct BusStat {
    double curve = 0.0;
    int route_length = 0;
    int stop_count = 0;
    int unique_stop_count = 0;
};

class RequestHandler {
public:
    RequestHandler(catalogue::TransportCatalogue& catalogue);

    void AddStop(const std::string& name, double lat, double lng);

    void AddDistance(const std::string& stop1, const std::string& stop2, int distance);

    void AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip);

    std::optional<domain::BusStat> GetBusStat(const std::string_view& bus_name) const;

    const std::set<std::string_view> GetBusesByStop(const std::string_view& stop_name) const;
    
    bool CheckStop(const std::string_view& stop_name);
    
    std::set<domain::Stop*> GetAllStops() const;

    const std::map<std::string_view, domain::Bus*>& GetAllBuses() const;
    
    const catalogue::TransportCatalogue& GetCatalogue() const;
    
    std::string_view GetStopToIndex (size_t id);
private:
    catalogue::TransportCatalogue& catalogue_;
};

}