#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>

#include "transport_catalogue.h"
#include "geo.h"
#include "request_handler.h"
#include "json_reader.h"
//#include "log_duration.h"
#include "map_renderer.h"

using namespace std;

using namespace std::literals;

int main() {

	ifstream input;
	input.open("input.json");

	request_handler::RequestsProcessing(input);

	//request_handler::RequestsProcessing(cin);

}