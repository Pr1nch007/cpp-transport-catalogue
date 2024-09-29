#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

using namespace std;

namespace parse{
/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
geo::Coordinates Coordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

vector<pair<int, string>> DistanceToStop (string_view str){
    vector<pair<int, string>> distance_to_stop;
    auto comma = str.find(',');

    while((comma = str.find_first_of(',', comma + 1)) != str.npos){
    auto first_dist = str.find_first_not_of(' ', comma + 1);
    auto metr = str.find_first_of('m', first_dist);
    
    auto first_stop = metr + 5;
    auto comma_2 = str.find_first_of(',', first_stop);
    
    int dist = std::stod(std::string(str.substr(first_dist, metr - first_dist)));
    std::string stop = std::string(str.substr(first_stop, comma_2 - first_stop));
    distance_to_stop.push_back({dist, stop});
    }
    return distance_to_stop;
}
    
namespace detail{
/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}
}
/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> Route(std::string_view route) {
    if (route.find('>') != route.npos) {
        return detail::Split(route, '>');
    }

    auto stops = detail::Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

reader::CommandDescription CommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}
}

namespace reader{

void InputReader::ParseLine(std::string_view line) {
    auto command_description = parse::CommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands([[maybe_unused]] catalogue::TransportCatalogue& catalogue) const {
    for(auto i : commands_){
        if(i.command == "Stop"s){
            catalogue::Stop stop;
            stop.name = std::move(i.id);
            stop.coord = std::move(parse::Coordinates(i.description));
            catalogue.AddStop(std::move(stop));
        }
    }
    for(auto i : commands_){
        if(i.command == "Stop"s){
            for(auto [dist, stop] : parse::DistanceToStop(i.description)){
                catalogue.AddDistance(i.id, stop, dist);
            }
        }
    }
    for(auto i : commands_){
        if(i.command == "Bus"s){
            catalogue::Bus bus;
            bus.name = std::move(i.id);
            
            auto stops = parse::Route(i.description);
            for(auto i : stops){
                catalogue::Stop* stop_ptr = catalogue.FindStop(i);
                bus.stops.push_back(std::move(stop_ptr));
            }
            
            catalogue.AddBus(std::move(bus));
        }
    }
}
}