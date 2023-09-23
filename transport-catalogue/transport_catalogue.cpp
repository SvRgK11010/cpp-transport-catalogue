#include "transport_catalogue.h"

using namespace std;

namespace catalogue {

	TransportCatalogue::TransportCatalogue()
	{}

	void TransportCatalogue::AddBus(const std::string& name, const std::vector<domain::Stop*>& stops, bool roundtrip) {
		domain::Bus bus(name, stops, roundtrip);
		all_buses_.push_back(bus);
		Bus* ref_bus = &all_buses_.back();
		busname_to_bus_.insert({ string_view{ref_bus->bus_number_}, ref_bus });
		for (auto& stop : stops) {
			stop->buses_on_stop_.insert(string_view{ ref_bus->bus_number_ });
		}
	}

	void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& geo) {
		domain::Stop stop((name), geo, all_stops_.size());
		all_stops_.push_back(stop);
		Stop* ref_stop = &(all_stops_.back());
		stopname_to_stop_.insert({ string_view{ref_stop->stop_name_} , ref_stop });
	}

	void TransportCatalogue::FillDistances(Stop* from, Stop* to, int met) {
		if (from != nullptr && to != nullptr) {
			distances_.insert({ {from, to}, met });
		}
	}

	int TransportCatalogue::ComputeDistanceToMeters(Stop* from, Stop* to) const {
		if (distances_.count({ from, to })) {
			return distances_.at({ from, to });
		}
		else if (distances_.count({ to, from })) {
			return distances_.at({ to, from });
		}
		else { return 0U; }
	}

	Stop* TransportCatalogue::GetStop(std::string_view stop_name) const {
		if (!stopname_to_stop_.count(stop_name)) {
			return nullptr;
		}
		return stopname_to_stop_.at(stop_name);
	}

	Bus* TransportCatalogue::GetBus(std::string_view bus_name) const {
		//assert(busname_to_bus_.count(bus_name) != 0);
		if (!busname_to_bus_.count(bus_name)) {
			return nullptr;
		}
		return busname_to_bus_.at(bus_name);
	}

	Stop* TransportCatalogue::GetStopForBus(std::string_view stop_name) {
		if (!stop_to_buses_.count(GetStop(stop_name))) { return nullptr; }
		return GetStop(stop_name);
	}

	std::vector<Bus*> TransportCatalogue::GetBusesForMap() const {
		std::vector<Bus*> result;
		for (auto& [bus_name, bus] : busname_to_bus_) {
			if (!bus->stops_.empty()) {
				result.push_back(bus);
			}
		}
		std::sort(result.begin(), result.end(), [](Bus* bus1, Bus* bus2) {return bus1->bus_number_ < bus2->bus_number_; });
		return result;
	}

	std::deque<Bus> TransportCatalogue::GetAllBuses() const {
		return all_buses_;
	}
	std::deque<Stop> TransportCatalogue::GetAllStops() const {
		return all_stops_;
	}

	int TransportCatalogue::GetDistance(Stop* from, Stop* to) const {
		if (distances_.count({ from, to })) {
			return distances_.at({ from, to });
		}
		else {
			return distances_.at({ to, from });
		}
	}

	std::unordered_map<std::pair<Stop*, Stop*>, int, StopHasher> TransportCatalogue::GetDistances() const {
		return distances_;
	}
}