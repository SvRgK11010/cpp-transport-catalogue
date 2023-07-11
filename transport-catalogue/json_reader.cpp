#include "json_reader.h"
namespace jsonreader {

	const json::Node& JsonReader::GetBaseRequests() const {
		auto pos = requests_.GetRoot().AsMap().find("base_requests");
		if (pos != requests_.GetRoot().AsMap().end()) {
			return pos->second;
		}
		else { throw std::out_of_range("No base requests"); }
	}
	const json::Node& JsonReader::GetStatRequests() const {
		auto pos = requests_.GetRoot().AsMap().find("stat_requests");
		if (pos != requests_.GetRoot().AsMap().end()) {
			return pos->second;
		}
		else { throw std::out_of_range("No stat requests"); }
	}

	const json::Node& JsonReader::GetRenderSettings() const {
		auto pos = requests_.GetRoot().AsMap().find("render_settings");
		if (pos != requests_.GetRoot().AsMap().end()) {
			return pos->second;
		}
		else { throw std::out_of_range("No render settings"); }
	}

	void JsonReader::FillCatalogue(catalogue::TransportCatalogue& catalogue) {
		const json::Array& all_requests = GetBaseRequests().AsArray();
		//добавление остановок
		for (auto& stop_r : all_requests) {
				const auto& stops_req = stop_r.AsMap();
				const auto& type = stops_req.at("type").AsString();
				if (type == "Stop") {
					//std::cout << "Type request:" << type << std::endl;
					StopData stop = SetStop(stops_req);
					catalogue.AddStop(stop.first, stop.second);
				}
			}
		
		//добавление дистанций
		for (auto& stop_r : all_requests) {
				const auto& stops_req = stop_r.AsMap();
				const auto& type = stops_req.at("type").AsString();
				if (type == "Stop") {
					DistData dist = SetDistances(stops_req, catalogue);
					for (auto& [stops, met] : dist) {
						catalogue.FillDistances(stops.first, stops.second, met);
					}
				}
			}
		
		//добавление автобусов
		for (auto& bus_r : all_requests) {
				const auto& buses_r = bus_r.AsMap();
				const auto& type = buses_r.at("type").AsString();
				if (type == "Bus") {

					BusData bus = SetBus(buses_r, catalogue);
					catalogue.AddBus(std::get<0>(bus), std::get<1>(bus), std::get<2>(bus));
				}
			}
		
	}

	const map_renderer::MapRenderer JsonReader::SetRenderSettings(const json::Dict& settings) const {
		map_renderer::RenderSettings rs_;
		rs_.width = settings.at("width").AsDouble();
		rs_.height = settings.at("height").AsDouble();
		rs_.padding = settings.at("padding").AsDouble();
		rs_.line_width = settings.at("line_width").AsDouble();
		rs_.stop_radius = settings.at("stop_radius").AsDouble();
		rs_.bus_label_font_size = settings.at("bus_label_font_size").AsInt();
		rs_.stop_label_font_size = settings.at("stop_label_font_size").AsInt();
		rs_.underlayer_width = settings.at("underlayer_width").AsDouble();

		const json::Array& bus_label_offset = settings.at("bus_label_offset").AsArray();
		rs_.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};
		const json::Array& stop_label_offset = settings.at("stop_label_offset").AsArray();
		rs_.stop_label_offset = { stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() };

		if (settings.at("underlayer_color").IsString()) {
			rs_.underlayer_color = settings.at("underlayer_color").AsString();
		}
		else if (settings.at("underlayer_color").IsArray()) {
			const json::Array& color = settings.at("underlayer_color").AsArray();
				if (color.size() == 3) { //rgb
					rs_.underlayer_color = svg::Rgb(color[0].AsInt(), color[1].AsInt(), color[2].AsInt());
				}
				else if (color.size() == 4) {
					rs_.underlayer_color = svg::Rgba(color[0].AsInt(), color[1].AsInt(), color[2].AsInt(), color[3].AsDouble());
				}
		}

		const json::Array& color_palette = settings.at("color_palette").AsArray();
		for (auto& color : color_palette) {
			if (color.IsString()) {
				rs_.color_palette.push_back(color.AsString());
			}
			else if (color.IsArray()) {
				const json::Array& c_ = color.AsArray();
				if (c_.size() == 3) { //rgb
					rs_.color_palette.push_back(svg::Rgb(c_[0].AsInt(), c_[1].AsInt(), c_[2].AsInt()));
				}
				else if (c_.size() == 4) {
					rs_.color_palette.push_back(svg::Rgba(c_[0].AsInt(), c_[1].AsInt(), c_[2].AsInt(), c_[3].AsDouble()));
				}
			}
		}
		map_renderer::MapRenderer result(rs_);
		return result;
	}

	json::Node JsonReader::ResponseToStatRequests(const json::Node& all_requests, const catalogue::TransportCatalogue& catalogue, const map_renderer::MapRenderer& map_renderer) const {
		json::Array output_data;
		for (auto& req : all_requests.AsArray()) {
			const auto& map_ = req.AsMap();
			const auto& type = map_.at("type").AsString();
			if (type == "Stop") {
				//std::cout << "Type:" << type << std::endl;
				//std::cout << "Stopname:" << map_.at("name").AsString() << std:: endl;
				output_data.push_back(FillStopInfo(map_, catalogue).AsMap());
			}
			else if (type == "Bus") {
				//std::cout << "Type:" << type << std::endl;
				output_data.push_back(FillBusInfo(map_, catalogue).AsMap());
			}
			else if (type == "Map") {
				output_data.push_back(FillMapInfo(map_, map_renderer, catalogue).AsMap());
			}
		}
		return output_data;
	}

	JsonReader::StopData JsonReader::SetStop(const json::Dict& stop_requests) {
		using namespace std::literals;
		std::string name = stop_requests.at("name").AsString();
		geo::Coordinates geo;
		geo.lat = stop_requests.at("latitude").AsDouble();
		geo.lng = stop_requests.at("longitude").AsDouble();
		return { name, geo };
	}
	JsonReader::BusData JsonReader::SetBus(const json::Dict& bus_requests, catalogue::TransportCatalogue& catalogue) {
		std::string name = bus_requests.at("name").AsString();		
		bool is_round = bus_requests.at("is_roundtrip").AsBool();		
		std::vector<domain::Stop*> stops;
		for (auto& stop : bus_requests.at("stops").AsArray()) {
			stops.push_back(catalogue.GetStop(stop.AsString()));
		}		
		return { name, stops, is_round };
	}
	JsonReader::DistData JsonReader::SetDistances(const json::Dict& stop_requests, catalogue::TransportCatalogue& catalogue) {
		std::string name = stop_requests.at("name").AsString();
		const json::Dict& stops = stop_requests.at("road_distances").AsMap();
		JsonReader::DistData result;
		for (auto& [stop2, dist] : stops) {
			std::pair from_to = { catalogue.GetStop(name), catalogue.GetStop(stop2) };
			result[from_to] = dist.AsInt();
		}
		return result;
	}

	const json::Node JsonReader::FillBusInfo(const json::Dict& bus_info, const catalogue::TransportCatalogue& catalogue) const {
		json::Dict result;
		result["request_id"] = bus_info.at("id").AsInt();
		//std::cout << "Bus_id: " << result["request_id"].AsInt() << std::endl;
		const std::string& name = bus_info.at("name").AsString();
		if (catalogue.GetBus(name) == nullptr) {
			result["error_message"] = json::Node{ static_cast<std::string>("not found") };
		}
		else {
			result["curvature"] = GetBusInfo(name, catalogue)->curvature;
			result["unique_stop_count"] = GetBusInfo(name, catalogue)->unique_stops_;
			result["route_length"] = GetBusInfo(name, catalogue)->m_route_length_;
			result["stop_count"] = GetBusInfo(name, catalogue)->stop_count_;
		}
		return json::Node{ result };

	}
	const json::Node JsonReader::FillStopInfo(const json::Dict& stop_info, const catalogue::TransportCatalogue& catalogue) const {
		json::Dict result;
		result["request_id"] = stop_info.at("id").AsInt();
		//std::cout << "Stop_id: " << result["request_id"].AsInt() << std::endl;
		const std::string& name = stop_info.at("name").AsString();
		//std::cout << "Stop_name:" << name << std::endl;
		if (catalogue.GetStop(name) == nullptr) {
			result["error_message"] = json::Node{ static_cast<std::string>("not found") };
		}
		else {
			json::Array buses;
			for (auto& bus : GetStopInfo(name, catalogue)) {
				buses.push_back(std::string{ bus });
			}
			//std::sort(buses.begin(), buses.end());
			result["buses"] = buses;
		}
		return json::Node{ result };
	}

	const json::Node JsonReader::FillMapInfo(const json::Dict& map_info, const map_renderer::MapRenderer& map_renderer, const catalogue::TransportCatalogue& catalogue) const {
		json::Dict result;
		result["request_id"] = map_info.at("id").AsInt();
		std::ostringstream out;
		svg::Document busmap = RenderMap(map_renderer, catalogue);
		busmap.Render(out);
		result["map"] = out.str();

		return json::Node{ result };
	}

	std::optional<domain::BusInfo> JsonReader::GetBusInfo(const std::string_view& bus_name, const catalogue::TransportCatalogue& catalogue) const {
		domain::BusInfo bus_info;
		domain::Bus* bus = catalogue.GetBus(bus_name);
		if (bus == nullptr) { throw std::invalid_argument("not found"); }
		bus_info.bus_numb_ = bus->bus_number_;
		if (bus->roundtrip_) {
			bus_info.stop_count_ = static_cast<int>(bus->stops_.size());
		}
		else if (!bus->roundtrip_) {
			bus_info.stop_count_ = static_cast<int>(bus->stops_.size() * 2 - 1);
		}
		std::unordered_set<std::string_view> stops;
		for (auto& stop : bus->stops_) {
			stops.insert(stop->stop_name_);
		}
		bus_info.unique_stops_ = static_cast<int>(stops.size());
		for (int i = 0; i < bus->stops_.size() - 1; i++) {
			if (bus->roundtrip_) { //кольцевой маршрут
				bus_info.geo_route_length_ += ComputeDistance(bus->stops_[i]->coordinates_, bus->stops_[i + 1]->coordinates_);
				bus_info.m_route_length_ += catalogue.ComputeDistanceToMeters(bus->stops_[i], bus->stops_[i + 1]);
			}
			else {//не кольцевой маршрут
				bus_info.geo_route_length_ += ComputeDistance(bus->stops_[i]->coordinates_, bus->stops_[i + 1]->coordinates_) * 2;
				bus_info.m_route_length_ += catalogue.ComputeDistanceToMeters(bus->stops_[i], bus->stops_[i + 1]) + catalogue.ComputeDistanceToMeters(bus->stops_[i + 1], bus->stops_[i]);
			}
		}
		bus_info.curvature = bus_info.m_route_length_ / bus_info.geo_route_length_;
		return bus_info;

	}

	const std::set<std::string_view> JsonReader::GetStopInfo(const std::string_view& stop_name, const catalogue::TransportCatalogue& catalogue) const {

		return catalogue.GetStop(stop_name)->buses_on_stop_;
	}

	svg::Document JsonReader::RenderMap(const map_renderer::MapRenderer& map_renderer, const catalogue::TransportCatalogue& catalogue) const {
		return map_renderer.RenderBuses(catalogue);
	}
}