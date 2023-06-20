#include <iostream>
#include <string>
#include <iomanip>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "geo.h"


using namespace std;

using namespace std::literals;

int main() {
	
	
	Catalogue::TransportCatalogue transport_catalogue;  

	Input::CreateTransportCatalogue(transport_catalogue);

	Output::PrintInfo(transport_catalogue);
	
}


