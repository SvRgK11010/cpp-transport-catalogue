#pragma once
#include <string>
#include <iostream>
#include <utility>
#include <unordered_map>
#include "transport_catalogue.h"
#include "geo.h"


namespace Input {
	using namespace Catalogue;

	std::string ReadLine();

	int ReadLineWithNumber();

	std::pair<std::string, std::string> SplitRequest(std::string& query);

	Stop ParseStop(std::string& query);

	std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopHasher> ParseDistances(std::string& query, TransportCatalogue& transport_cat);

	Bus ParseBus(std::string& query, TransportCatalogue& transport_cat);

	void CreateTransportCatalogue(TransportCatalogue& transport_cat);
}