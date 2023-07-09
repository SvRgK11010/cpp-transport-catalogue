#include "json_reader.h"
namespace jsonreader {

	const json::Node& JsonReader::GetBaseRequests() const {
		assert(requests_.GetRoot().AsMap().count("base_requests"));
		return requests_.GetRoot().AsMap().at("base_requests");
	}
	const json::Node& JsonReader::GetStatRequests() const {
		assert(requests_.GetRoot().AsMap().count("stat_requests"));
		return requests_.GetRoot().AsMap().at("stat_requests");
	}

	const json::Node& JsonReader::GetRenderSettings() const {
		assert(requests_.GetRoot().AsMap().count("render_settings"));
		return requests_.GetRoot().AsMap().at("render_settings");
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

	JsonReader::StopData JsonReader::SetStop(const json::Dict& stop_requests) {
		using namespace std::literals;
		std::string name = stop_requests.at("name").AsString();
		geo::Coordinates geo;
		geo.lat = stop_requests.at("latitude").AsDouble();
		geo.lng = stop_requests.at("longitude").AsDouble();
		return std::make_pair(name, geo);
	}
	JsonReader::BusData JsonReader::SetBus(const json::Dict& bus_requests, catalogue::TransportCatalogue& catalogue) {
		std::string name = bus_requests.at("name").AsString();		
		bool is_round = bus_requests.at("is_roundtrip").AsBool();		
		std::vector<domain::Stop*> stops;
		for (auto& stop : bus_requests.at("stops").AsArray()) {
			stops.push_back(catalogue.GetStop(stop.AsString()));
		}
		return std::make_tuple(name, stops, is_round);
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
}