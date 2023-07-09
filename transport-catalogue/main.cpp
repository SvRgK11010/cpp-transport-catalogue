#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>

#include "transport_catalogue.h"
#include "geo.h"
#include "request_handler.h"
#include "json_reader.h"
#include "log_duration.h"
#include "map_renderer.h"

using namespace std;

using namespace std::literals;

int main() {
	catalogue::TransportCatalogue catalogue1;

	ifstream input;
	input.open("input.json");
	
	jsonreader::JsonReader requests(input);
		
	requests.FillCatalogue(catalogue1); 
	
	const auto& render_settings = requests.GetRenderSettings().AsMap();
	const auto& render = requests.SetRenderSettings(render_settings);

	RequestHandler out(catalogue1, render);

	//const auto& sort_buses = catalogue1.GetBusesForMap();
	//std::cout << "Buses:" << endl;
	//for (auto& bus : sort_buses) {
	//	std::cout << bus->bus_number_ << std::endl;
	//}
	const auto& stat_requests = requests.GetStatRequests();
	out.RequestsProcessing(stat_requests);


	//out.RenderMap().Render(std::cout);

}

