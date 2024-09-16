#include "stat_reader.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <string>


using namespace std;

namespace print{

namespace detail{ 

void PrintInfoBus(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view name_bus,
                       std::ostream& output){
    if(tansport_catalogue.FindBus(name_bus) == nullptr){
        output << "Bus "s << name_bus << ": not found"s << endl;
        return;
    }
    
    catalogue::Bus bus = *tansport_catalogue.FindBus(name_bus);
    double dist = 0.0;
    set<string_view> unique_stop;
    
    for(size_t i = 0; i < bus.stops.size(); ++i){
        unique_stop.insert(bus.stops[i]->name);
        
        if(i == 0){
            continue;
        }
        
        dist += geo::ComputeDistance(bus.stops[i-1]->coord, bus.stops[i]->coord);
    }
    
    output << "Bus "s << name_bus << ": "s << bus.stops.size() << " stops on route, "s << unique_stop.size() << " unique stops, "s << dist << " route length"s << endl;
}
    
void PrintInfoStop(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view name_stop,
                       std::ostream& output){
    auto buses_to_stop = tansport_catalogue.GetStopInfo(name_stop);
        
    if(buses_to_stop.empty()){
        if(tansport_catalogue.FindStop(name_stop) == nullptr){
            output << "Stop "s << name_stop << ": not found"s << endl;
        }else{
            output << "Stop "s << name_stop << ": no buses"s << endl;
        }
        return;
    }
        
    output << "Stop "s << name_stop << ": buses"s;
        
    for(auto& bus_name : buses_to_stop){
        output << " "s << bus_name;
    }
        
    output << endl;
}
}
    
void ParseAndPrintStat(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output) {
    auto colon_pos = request.find_first_not_of(' ');
    auto space_pos = request.find(' ');
    auto not_space = request.find_first_not_of(' ', space_pos);
    
    string command = move(string(request.substr(0, space_pos)));
    string name_bus_or_stop = move(string(request.substr(not_space, colon_pos - not_space + 1)));
    if(command == "Bus"s){
        detail::PrintInfoBus(tansport_catalogue, name_bus_or_stop, output);
    }else{
        detail::PrintInfoStop(tansport_catalogue, name_bus_or_stop, output);
    }
}
}