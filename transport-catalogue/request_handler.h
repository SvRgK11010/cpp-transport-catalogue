#pragma once
#include <optional>
#include <iostream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"

namespace request_handler {

	void RequestsProcessing(std::istream& input);

}