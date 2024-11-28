#include <iostream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"

int main(){
    catalogue::TransportCatalogue catalogue;
    map_renderer::MapRenderer renderer;
    handler::RequestHandler handler(catalogue);
    router::TransportRouter transport_router;
    reader::JsonHandler json_handler(std::cin, handler, renderer, transport_router);

    json_handler.ProcessInput();
    json_handler.ProcessOutput(std::cout);
    
    return 0;
}