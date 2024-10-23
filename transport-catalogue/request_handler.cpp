#include "request_handler.h"

#include "geo.h"

namespace handler{
    
    RequestHandler::RequestHandler(catalogue::TransportCatalogue& catalogue) 
        : catalogue_(catalogue) {}

    void RequestHandler::AddStop(const std::string& name, double lat, double lng) {
        catalogue_.AddStop({name, {lat, lng}});
    }

    void RequestHandler::AddDistance(const std::string& stop1, const std::string& stop2, int distance) {
        catalogue_.AddDistance(stop1, stop2, distance);
    }

    void RequestHandler::AddBus(const std::string& name, const std::vector<std::string>& stops, bool is_roundtrip) {
        catalogue_.AddBus(name, stops, is_roundtrip);
    }

    std::optional<domain::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
        return catalogue_.GetBusInfo(bus_name);
    }

    const std::set<std::string_view> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
        if(catalogue_.FindStop(stop_name) == nullptr) {
            return {};
        }
        
        return catalogue_.GetStopInfo(stop_name);
    }
    
    bool RequestHandler::CheckStop(const std::string_view& stop_name) {
        if(catalogue_.FindStop(stop_name) == nullptr) {
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
