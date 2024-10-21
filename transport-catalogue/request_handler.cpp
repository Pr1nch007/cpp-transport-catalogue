#include "request_handler.h"

#include "geo.h"

namespace handler{
    
    RequestHandler::RequestHandler(catalogue::TransportCatalogue& catalogue) 
        : catalogue_(catalogue) {}

    void RequestHandler::AddStop(const std::string& name, double lat, double lng) {
        catalogue_.AddStop({name, {lat, lng}});
    }

    void RequestHandler::AddDistance(const std::string& stop1, const std::string& stop2, int distance){
        catalogue_.AddDistance(stop1, stop2, distance);
    }

    void RequestHandler::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip) {
        catalogue::Bus bus;
        bus.name = name;
        
        for (const auto& stop_name : stops) {
            domain::Stop* stop = catalogue_.FindStop(stop_name);
            bus.stops.push_back(stop);
        }
        
        bus.is_roundtrip = is_roundtrip;
        catalogue_.AddBus(bus);
    }

    std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const{
        if(catalogue_.FindBus(bus_name) == nullptr){
            return {};
        }

        domain::Bus bus = *catalogue_.FindBus(bus_name);
        int fact_dist = 0;
        double geo_dist = 0.0;
        std::set<std::string_view> unique_stop;

        for(size_t i = 0; i < bus.stops.size(); ++i){
            unique_stop.insert(bus.stops[i]->name);
    
            if(i == 0){
                continue;
            }
    
            geo_dist += geo::ComputeDistance(bus.stops[i-1]->coord, bus.stops[i]->coord);
            fact_dist += catalogue_.FindDistance(bus.stops[i-1]->name, bus.stops[i]->name);
        }

        double curv = static_cast<double>(fact_dist) / geo_dist;
        return std::optional<BusStat>{{curv, fact_dist, static_cast<int>(bus.stops.size()), static_cast<int>(unique_stop.size())}};
    }

    const std::set<std::string_view> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const{
        if(catalogue_.FindStop(stop_name) == nullptr){
            return {};
        }
        
        return catalogue_.GetStopInfo(stop_name);
    }
    
    bool RequestHandler::YesOrNoStop(const std::string_view& stop_name) {
        if(catalogue_.FindStop(stop_name) == nullptr){
            return false;
        }
        
        return true;
    }
    
    std::set<domain::Stop*> RequestHandler::GetAllStops() const {
        return  catalogue_.GetStopsInRoutes();
    }

    const std::map<std::string_view, domain::Bus*>& RequestHandler::GetAllBuses() const {
        return catalogue_.GetAllBuses();
    }
    
}
