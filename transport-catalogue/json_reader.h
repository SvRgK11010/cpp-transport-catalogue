#pragma once
#include <iostream>
#include <utility>
#include "json.h"
#include "transport_catalogue.h"
#include "geo.h"
#include "log_duration.h"
#include "map_renderer.h"

namespace jsonreader {
	
	class JsonReader {
		using StopData = std::pair<std::string, geo::Coordinates>;
		using BusData = std::tuple <std::string, std::vector<domain::Stop*>, bool>;
		using DistData = std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, int, catalogue::StopHasher>;

	public:
		JsonReader(std::istream& input)
			: requests_(json::Load(input))
		{}
		const json::Node& GetBaseRequests() const;
		const json::Node& GetStatRequests() const;
		const json::Node& GetRenderSettings() const;

		void FillCatalogue(catalogue::TransportCatalogue& catalogue);
		const map_renderer::MapRenderer SetRenderSettings(const json::Dict& settings) const;

	private:
		json::Document requests_;

		StopData SetStop(const json::Dict& stop_requests);
		BusData SetBus(const json::Dict& bus_recuests, catalogue::TransportCatalogue& catalogue);
		DistData SetDistances(const json::Dict& stop_requests, catalogue::TransportCatalogue& catalogue);
		
	};
}