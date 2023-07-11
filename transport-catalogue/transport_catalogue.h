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
#include <stdexcept>
#include <set>


#include "geo.h"
#include "domain.h"

namespace catalogue {
	using namespace geo;
	using namespace domain;


	struct StopHasher {
		size_t operator()(std::pair<const Stop*, const Stop*> stops) const {
			return stop_hash(static_cast<const void*>(stops.first)) * 41 * 41 + stop_hash(static_cast<const void*>(stops.second)) * 41;
		}

		size_t operator()(const Stop* stop) const {
			return stop_hash(static_cast<const void*>(stop)) * 41;
		}

	private:
		std::hash<const void*> stop_hash;
	};


	class TransportCatalogue {
	public:

		TransportCatalogue();

		void AddBus(const std::string& name, const std::vector<domain::Stop*>& stops, bool roundtrip);

		void AddStop(const std::string& name, const geo::Coordinates& geo);

		void FillDistances(Stop* from, Stop* to, int met);

		Stop* GetStop(std::string_view stop_name) const;

		Bus* GetBus(std::string_view bus_name) const;

		Stop* GetStopForBus(std::string_view stop_name);

		int ComputeDistanceToMeters(Stop* from, Stop* to) const;

		std::vector<Bus*> GetBusesForMap() const;


	private:
		std::deque<Stop> all_stops_;
		std::deque<Bus> all_buses_;
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<Stop*, std::unordered_set<Bus*>, StopHasher> stop_to_buses_;
		std::unordered_map<std::pair<Stop*, Stop*>, int, StopHasher> distances_;
	};
}