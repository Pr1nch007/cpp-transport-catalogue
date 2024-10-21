#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace domain{

struct Stop{
    std::string name;
    geo::Coordinates coord;
};

struct Bus{
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};    
    
}