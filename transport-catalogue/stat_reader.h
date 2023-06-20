#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <iomanip>
#include "transport_catalogue.h"
#include "input_reader.h"
#include "geo.h"

namespace Output {
	using namespace Catalogue;
	using namespace Geo;
	using namespace Input;

	void PrintInfo(TransportCatalogue& transport_catalogue);

}