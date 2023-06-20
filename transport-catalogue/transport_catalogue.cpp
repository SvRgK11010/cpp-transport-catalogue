#include "transport_catalogue.h"

using namespace std;

namespace Catalogue {

	TransportCatalogue::TransportCatalogue()
	{}

	void TransportCatalogue::AddBus(const Bus& bus) {
		all_buses_.push_back(bus);
		Bus* ref_bus = &all_buses_.back();
		busname_to_bus_.insert({ string_view{ref_bus->bus_number_}, ref_bus });
		for (size_t i = 0; i < bus.stops_.size(); i++) {
			stop_to_buses_[bus.stops_[i]].insert(ref_bus);
		}
	}

	void TransportCatalogue::AddStop(const Stop& stop) {
		all_stops_.push_back(stop);
		Stop* ref_stop = &(all_stops_.back());
		stopname_to_stop_.insert({ string_view{ref_stop->stop_name_} , ref_stop });
	}

	void TransportCatalogue::FillDistances(Stop* from, Stop* to, size_t met) {
		if (from != nullptr && to != nullptr) {
			distances_.insert({ {from, to}, met });
		}
	}

	size_t TransportCatalogue::ComputeDistanceToMeters(Stop* from, Stop* to) {
		if (distances_.count({ from, to })) {
			return distances_.at({ from, to });
		}
		else if (distances_.count({ to, from })) {
			return distances_.at({ to, from });
		}
		else { return 0U; }
	}

	Stop* TransportCatalogue::GetStop(std::string_view stop_name) {
		if (!stopname_to_stop_.count(stop_name)) { return nullptr; }
		return stopname_to_stop_.at(stop_name);
	}

	Bus* TransportCatalogue::GetBus(std::string_view bus_name) {
		//assert(busname_to_bus_.count(bus_name) != 0);
		if (!busname_to_bus_.count(bus_name)) { return nullptr; }
		return busname_to_bus_.at(bus_name);
	}

	BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) {
		BusInfo bus_info;
		//if (busname_to_bus_.count(bus_name))
		//{
		bus_info.bus_numb_ = bus_name;
		bus_info.stop_count_ = static_cast<int>(busname_to_bus_.at(bus_name)->stops_.size());

		auto temp_stops = busname_to_bus_.at(bus_name)->stops_;
		sort(temp_stops.begin(), temp_stops.end());
		auto unique_stops = unique(temp_stops.begin(), temp_stops.end());
		if (unique_stops != temp_stops.end()) {
			temp_stops.erase(unique_stops, temp_stops.end());
		}
		bus_info.unique_stops_ = static_cast<int>(temp_stops.size());
		if (bus_info.stop_count_ > 1) {
			auto& bus = busname_to_bus_.at(bus_name);
			for (int i = 0; i < bus_info.stop_count_ - 1; i++) {
				bus_info.geo_route_length_ += ComputeDistance(bus->stops_[i]->coordinates_, bus->stops_[i + 1]->coordinates_);
				bus_info.m_route_length_ += ComputeDistanceToMeters(bus->stops_[i], bus->stops_[i + 1]);
			}
		}
		bus_info.curvature = bus_info.m_route_length_ / bus_info.geo_route_length_;
		//}
		return bus_info;
	}

	StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) {
		StopInfo stop_info;

		stop_info.stop_name_ = stop_name;
		for (auto bus : stop_to_buses_.at(GetStop(stop_name))) {
			stop_info.buses_on_stop_.push_back(bus->bus_number_);
		}
		sort(stop_info.buses_on_stop_.begin(), stop_info.buses_on_stop_.end());

		return stop_info;
	}

	Stop* TransportCatalogue::GetStopForBus(std::string_view stop_name) {
		if (!stop_to_buses_.count(GetStop(stop_name))) { return nullptr; }
		return GetStop(stop_name);
	}


	int TransportCatalogue::GetStopCount() {
		return static_cast<int>(all_stops_.size());
	}

	int TransportCatalogue::GetBusCount() {
		return static_cast<int>(all_buses_.size());
	}

	std::deque<Stop> TransportCatalogue::PrintStops() {
		return all_stops_;
	}

	std::deque<Bus> TransportCatalogue::PrintBuses() {
		return all_buses_;
	}

	std::unordered_map<std::string_view, Stop*> TransportCatalogue::PrintStopMap() {
		return stopname_to_stop_;
	}

	std::unordered_map<std::string_view, Bus*> TransportCatalogue::PrintBusMap() {
		return busname_to_bus_;
	}

	std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopHasher> TransportCatalogue::GetDistMap() {
		return distances_;
	}
}