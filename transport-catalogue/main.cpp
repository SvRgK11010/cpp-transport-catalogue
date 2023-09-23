#include <fstream>
#include <iostream>
#include <string_view>
#include <iostream>
#include <string>
#include <iomanip>


#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"
#include "serialization.h"


using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        request_handler::RequestsProcessingMakeBase(std::cin);

    }
    else if (mode == "process_requests"sv) {

        request_handler::RequestsProcessingMakeAnswers(std::cin);
    
    } else {
        PrintUsage();
        return 1;
    }
}