#include "request_handler.h"

using namespace catalogue;
using namespace jsonreader;

void RequestHandler::RequestsProcessing(const json::Node& all_requests) const {
	json::Array output_data;
	for (auto& req : all_requests.AsArray()) {
		const auto& map_ = req.AsMap();
		const auto& type = map_.at("type").AsString();
		if (type == "Stop") {
			//std::cout << "Type:" << type << std::endl;
			//std::cout << "Stopname:" << map_.at("name").AsString() << std:: endl;
			output_data.push_back(FillStopInfo(map_).AsMap());
		}
		else if (type == "Bus") {
			//std::cout << "Type:" << type << std::endl;
			output_data.push_back(FillBusInfo(map_).AsMap());
		}
		else if (type == "Map") {
			output_data.push_back(FillMapInfo(map_).AsMap());
		}
	}
	json::Print(json::Document{ output_data }, std::cout);

}

const json::Node RequestHandler::FillBusInfo(const json::Dict& bus_info) const {
	json::Dict result;
	result["request_id"] = bus_info.at("id").AsInt();
	//std::cout << "Bus_id: " << result["request_id"].AsInt() << std::endl;
	const std::string& name = bus_info.at("name").AsString();
	if (db_.GetBus(name) == nullptr) {
		result["error_message"] = json::Node{ static_cast<std::string>("not found") };
	}
	else {
		result["curvature"] = GetBusInfo(name)->curvature;
		result["unique_stop_count"] = GetBusInfo(name)->unique_stops_;
		result["route_length"] = GetBusInfo(name)->m_route_length_;
		result["stop_count"] = GetBusInfo(name)->stop_count_;
	}
	return json::Node{ result };

}
const json::Node RequestHandler::FillStopInfo(const json::Dict& stop_info) const {
	json::Dict result;
	result["request_id"] = stop_info.at("id").AsInt();
	//std::cout << "Stop_id: " << result["request_id"].AsInt() << std::endl;
	const std::string& name = stop_info.at("name").AsString();
	//std::cout << "Stop_name:" << name << std::endl;
	if (db_.GetStop(name) == nullptr) {
		result["error_message"] = json::Node{ static_cast<std::string>("not found") };
	}
	else {
		json::Array buses;
		for (auto& bus : GetStopInfo(name)) {
			buses.push_back(std::string{ bus });
		}
		//std::sort(buses.begin(), buses.end());
		result["buses"] = buses;
	}
	return json::Node{ result };
}

const json::Node RequestHandler::FillMapInfo(const json::Dict& map_info) const {
	json::Dict result;
	result["request_id"] = map_info.at("id").AsInt();
	std::ostringstream out;
	svg::Document busmap = RenderMap();
	busmap.Render(out);
	result["map"] = out.str();

	return json::Node{ result };
}

std::optional<domain::BusInfo> RequestHandler::GetBusInfo(const std::string_view& bus_name) const {
	domain::BusInfo bus_info;
	domain::Bus* bus = db_.GetBus(bus_name);
	if (bus == nullptr) { throw std::invalid_argument("not found"); }
	bus_info.bus_numb_ = bus->bus_number_;
	if (bus->roundtrip_) { bus_info.stop_count_ = static_cast<int>(bus->stops_.size()); 
	}
	else if (!bus->roundtrip_ ) { bus_info.stop_count_ = static_cast<int>(bus->stops_.size()*2 -1);
	}
	std::unordered_set<std::string_view> stops;
	for (auto& stop : bus->stops_) {
		stops.insert(stop->stop_name_);
	}
	bus_info.unique_stops_ = static_cast<int>(stops.size());
	for (int i = 0; i<bus->stops_.size()-1; i++) {
		if (bus->roundtrip_) { //????????? ???????
			bus_info.geo_route_length_ += ComputeDistance(bus->stops_[i]->coordinates_, bus->stops_[i + 1]->coordinates_);
			bus_info.m_route_length_ += db_.ComputeDistanceToMeters(bus->stops_[i], bus->stops_[i + 1]);
		}
		else {//?? ????????? ???????
			bus_info.geo_route_length_ += ComputeDistance(bus->stops_[i]->coordinates_, bus->stops_[i + 1]->coordinates_)*2;
			bus_info.m_route_length_ += db_.ComputeDistanceToMeters(bus->stops_[i], bus->stops_[i + 1])+ db_.ComputeDistanceToMeters(bus->stops_[i+1], bus->stops_[i]);
		}
	}
	bus_info.curvature = bus_info.m_route_length_ / bus_info.geo_route_length_;
	return bus_info;

}

const std::set<std::string_view> RequestHandler::GetStopInfo(const std::string_view& stop_name) const {

	return db_.GetStop(stop_name)->buses_on_stop_;
}

svg::Document RequestHandler::RenderMap() const {
	return renderer_.RenderBuses(db_);
}