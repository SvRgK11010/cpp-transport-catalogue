#pragma once
#include <iostream>
#include <utility>
#include <sstream>
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
		//Формирование ответа на запрос к базе
		json::Node ResponseToStatRequests(const json::Node& all_requests, const catalogue::TransportCatalogue& catalogue, const map_renderer::MapRenderer& map_renderer) const;

	private:
		json::Document requests_;

		StopData SetStop(const json::Dict& stop_requests);
		BusData SetBus(const json::Dict& bus_recuests, catalogue::TransportCatalogue& catalogue);
		DistData SetDistances(const json::Dict& stop_requests, catalogue::TransportCatalogue& catalogue);

		const json::Node FillBusInfo(const json::Dict& bus_info, const catalogue::TransportCatalogue& catalogue) const;
		const json::Node FillStopInfo(const json::Dict& stop_info, const catalogue::TransportCatalogue& catalogue) const;
		const json::Node FillMapInfo(const json::Dict& map_info, const map_renderer::MapRenderer& map_renderer, const catalogue::TransportCatalogue& catalogue) const;

		// Возвращает информацию о маршруте (запрос Bus)
		std::optional<domain::BusInfo> GetBusInfo(const std::string_view& bus_name, const catalogue::TransportCatalogue& catalogue) const;

		// Возвращает маршруты, проходящие через (запрос Stop)
		const std::set<std::string_view> GetStopInfo(const std::string_view& stop_name, const catalogue::TransportCatalogue& catalogue) const;

		svg::Document RenderMap(const map_renderer::MapRenderer& map_renderer, const catalogue::TransportCatalogue& catalogue) const;
		
	};
}