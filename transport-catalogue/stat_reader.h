#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace print{
    
namespace detail{
    
void PrintInfoBus(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view name_bus,
                       std::ostream& output);

void PrintInfoStop(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view name_stop,
                       std::ostream& output);
}  
    
void ParseAndPrintStat(const catalogue::TransportCatalogue& tansport_catalogue, std::string_view request,
                       std::ostream& output);
}