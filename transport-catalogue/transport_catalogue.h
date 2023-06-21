#pragma once
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <string>
#include <utility>
#include <cassert>
#include <algorithm>


#include "geo.h"

namespace Catalogue {
	using namespace Geo;

	struct Stop {
		std::string name_;
		Coordinates coordinates_;

		explicit Stop(std::string name, Coordinates coord)
			: name_(name), coordinates_(coord)
		{
		}
	};

	struct StopHasher {
		size_t operator()(std::pair<const Stop*, const Stop*> stops) const {
			return stop_hash(static_cast<const void*>(stops.first)) * 41 + stop_hash(static_cast<const void*>(stops.second));
		}

		size_t operator()(const Stop* stop) const {
			return stop_hash(static_cast<const void*>(stop)) * 41;
		}

	private:
		std::hash<const void*> stop_hash;
	};

	struct Bus {
		std::string bus_number_;
		std::vector<Stop*> stops_ = {};

	};

	struct BusInfo {
		std::string_view bus_numb_;
		int stop_count_ = 0;
		int unique_stops_ = 0;
		double geo_route_length_ = 0.0;
		int m_route_length_ = 0;
		double curvature = 0.0;
	};

	struct StopInfo {
		std::string_view stop_name_;
		std::vector<std::string_view> buses_on_stop_;
	};

	class TransportCatalogue {
	public:

		TransportCatalogue();

		void AddBus(const Bus& bus);

		void AddStop(const Stop& stop);

		void SetDistances(Stop* from, Stop* to, size_t met);

		Stop* GetStop(std::string_view stop_name);

		Bus* GetBus(std::string_view bus_name);

		BusInfo GetBusInfo(std::string_view bus_name);

		StopInfo GetStopInfo(std::string_view stop_name);

		Stop* GetStopForBus(std::string_view stop_name);

		size_t GetDistanceToMeters(Stop* from, Stop* to);

		int GetStopCount();

		int GetBusCount();


		//Данные функции (90-98) использовались при тестировании других методов каталога, чтобы проверить, что именно записалось в поля класса
		std::deque<Stop> GetAllStops();

		std::deque<Bus> GetAllBuses();

		std::unordered_map<std::string_view, Stop*> GetStopMap();

		std::unordered_map<std::string_view, Bus*> GetBusMap();

		std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopHasher> GetDistMap();

	private:
		std::deque<Stop> all_stops_;
		std::deque<Bus> all_buses_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<Stop*, std::unordered_set<Bus*>, StopHasher> stop_to_buses_;
		std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopHasher> distances_;
	};
}