#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coord;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    bool is_roundtrip;
};    
    
struct BusStat {
    double curve = 0.0;
    int route_length = 0;
    int stop_count = 0;
    int unique_stop_count = 0;
};

}