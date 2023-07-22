#include "json_reader.h"
#include "json_builder.h"
namespace jsonreader {

	const json::Node& JsonReader::GetBaseRequests() const {
		auto pos = requests_.GetRoot().AsDict().find("base_requests");
		if (pos != requests_.GetRoot().AsDict().end()) {
			return pos->second;
		}
		else { throw std::out_of_range("No base requests"); }
	}
	const json::Node& JsonReader::GetStatRequests() const {
		auto pos = requests_.GetRoot().AsDict().find("stat_requests");
		if (pos != requests_.GetRoot().AsDict().end()) {
			return pos->second;
		}
		else { throw std::out_of_range("No stat requests"); }
	}

	const json::Node& JsonReader::GetRenderSettings() const {
		auto pos = requests_.GetRoot().AsDict().find("render_settings");
		if (pos != requests_.GetRoot().AsDict().end()) {
			return pos->second;
		}
		else { throw std::out_of_range("No render settings"); }
	}

	void JsonReader::FillCatalogue(catalogue::TransportCatalogue& catalogue) {
		const json::Array& all_requests = GetBaseRequests().AsArray();
		//добавление остановок
		for (auto& stop_r : all_requests) {
			const auto& stops_req = stop_r.AsDict();
			const auto& type = stops_req.at("type").AsString();
			if (type == "Stop") {
				StopData stop = SetStop(stops_req);
				catalogue.AddStop(stop.first, stop.second);
			}
		}

		//добавление дистанций
		for (auto& stop_r : all_requests) {
			const auto& stops_req = stop_r.AsDict();
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
			const auto& buses_r = bus_r.AsDict();
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
		rs_.bus_label_offset = { bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() };
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
			const auto& map_ = req.AsDict();
			const auto& type = map_.at("type").AsString();
			if (type == "Stop") {
				output_data.push_back(FillStopInfo(map_, catalogue).AsDict());
			}
			else if (type == "Bus") {
				output_data.push_back(FillBusInfo(map_, catalogue).AsDict());
			}
			else if (type == "Map") {
				output_data.push_back(FillMapInfo(map_, map_renderer, catalogue).AsDict());
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
		const json::Dict& stops = stop_requests.at("road_distances").AsDict();
		JsonReader::DistData result;
		for (auto& [stop2, dist] : stops) {
			std::pair from_to = { catalogue.GetStop(name), catalogue.GetStop(stop2) };
			result[from_to] = dist.AsInt();
		}
		return result;
	}

	const json::Node JsonReader::FillBusInfo(const json::Dict& bus_info, const catalogue::TransportCatalogue& catalogue) const {
		json::Node result;
		const int id_ = bus_info.at("id").AsInt();
		const std::string& name = bus_info.at("name").AsString();
		if (catalogue.GetBus(name) == nullptr) {
			result = json::Builder{}.StartDict()
				.Key("request_id").Value(id_)
				.Key("error_message").Value("not found")
				.EndDict()
				.Build();
		}
		else {
			result = json::Builder{}.StartDict()
				.Key("request_id").Value(id_)
				.Key("curvature").Value(GetBusInfo(name, catalogue)->curvature)
				.Key("unique_stop_count").Value(GetBusInfo(name, catalogue)->unique_stops_)
				.Key("route_length").Value(GetBusInfo(name, catalogue)->m_route_length_)
				.Key("stop_count").Value(GetBusInfo(name, catalogue)->stop_count_)
				.EndDict()
				.Build();
		}
		return result;

	}
	const json::Node JsonReader::FillStopInfo(const json::Dict& stop_info, const catalogue::TransportCatalogue& catalogue) const {
		json::Node result;
		const int id_ = stop_info.at("id").AsInt();
		const std::string& name = stop_info.at("name").AsString();
		if (catalogue.GetStop(name) == nullptr) {
			result = json::Builder{}.StartDict()
				.Key("request_id").Value(id_)
				.Key("error_message").Value("not found")
				.EndDict()
				.Build(); 
		}
		else {
			json::Array buses;
			for (auto& bus : GetStopInfo(name, catalogue)) {
				buses.push_back(std::string{ bus });
			}
			result = json::Builder{}.StartDict()
				.Key("request_id").Value(id_)
				.Key("buses").Value(buses)
				.EndDict()
				.Build();
		}
		return result;
	}

	const json::Node JsonReader::FillMapInfo(const json::Dict& map_info, const map_renderer::MapRenderer& map_renderer, const catalogue::TransportCatalogue& catalogue) const {
		json::Node result;
		const int id_ = map_info.at("id").AsInt();
		std::ostringstream out;
		svg::Document busmap = RenderMap(map_renderer, catalogue);
		busmap.Render(out);
		result = json::Builder{}.StartDict()
			.Key("request_id").Value(id_)
			.Key("map").Value(out.str())
			.EndDict()
			.Build();
		return result;
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
		for (size_t i = 0; i < bus->stops_.size() - 1; i++) {
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