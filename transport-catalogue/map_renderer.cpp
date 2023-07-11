#include "map_renderer.h"

namespace map_renderer {

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	BusColor DistributeColors(const std::vector<domain::Bus*>& buses, const std::vector<svg::Color>& color_palette) {
		BusColor colors;
		int color = 0;
		for (const auto& bus : buses) {
			colors.buscolors.insert({ bus->bus_number_, color });
			if (color < (color_palette.size() - 1)) { color++; }
			else { color = 0; }
		}
		return colors;
	}

	int FindBusColor(const BusColor& bus_color, const std::string_view& bus_name) {
		return bus_color.buscolors.at(bus_name);
	}

	svg::Polyline MapRenderer::VisualizeBusLines(const domain::Bus* bus, SphereProjector projector, const BusColor& colors) const {
		svg::Polyline bus_line;
		std::vector<domain::Stop*> stops = bus->stops_;
		if (!bus->roundtrip_) {
			stops.insert(stops.end(), std::next(bus->stops_.rbegin()), bus->stops_.rend());
		}
		for (auto& stop : stops) {
			bus_line.AddPoint(projector(stop->coordinates_));
		}
		bus_line.SetFillColor("none");
		bus_line.SetStrokeWidth(render_settings_.line_width);
		bus_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		bus_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		bus_line.SetStrokeColor(render_settings_.color_palette[FindBusColor(colors, bus->bus_number_)]);

		return bus_line;
	}

	std::vector<svg::Text> MapRenderer::VisualizeBusName(const domain::Bus* bus, SphereProjector projector, const BusColor& colors) const {
		svg::Text begin;
		svg::Text begin_u;
		std::vector<svg::Text> result;
		//общие настройки
		begin.SetPosition(projector(bus->stops_[0]->coordinates_));		
		begin.SetOffset(render_settings_.bus_label_offset);		
		begin.SetFontSize(render_settings_.bus_label_font_size);
		begin.SetFontFamily("Verdana");
		begin.SetFontWeight("bold");
		begin.SetData(bus->bus_number_);
		begin.SetFillColor(render_settings_.color_palette[FindBusColor(colors, bus->bus_number_)]);

		//настройки подложки
		begin_u.SetPosition(projector(bus->stops_[0]->coordinates_));
		begin_u.SetOffset(render_settings_.bus_label_offset);
		begin_u.SetFontSize(render_settings_.bus_label_font_size);
		begin_u.SetFontFamily("Verdana");
		begin_u.SetFontWeight("bold");
		begin_u.SetData(bus->bus_number_);
		begin_u.SetFillColor(render_settings_.underlayer_color);
		begin_u.SetStrokeColor(render_settings_.underlayer_color);
		begin_u.SetStrokeWidth(render_settings_.underlayer_width);
		begin_u.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		begin_u.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		result.push_back(begin_u);
		result.push_back(begin);
		//настройки для некольцевого маршрута
		if (!bus->roundtrip_ && bus->stops_[0] != bus->stops_[bus->stops_.size() - 1]) {
			svg::Text end{ begin };
			svg::Text end_u{ begin_u };
			end.SetPosition(projector(bus->stops_[bus->stops_.size() - 1]->coordinates_));
			end_u.SetPosition(projector(bus->stops_[bus->stops_.size() - 1]->coordinates_));
			
			result.push_back(end_u);
			result.push_back(end);
		}

		return result;
	}

	std::vector<svg::Circle> MapRenderer::VisualizeStopPoints(const std::map<std::string_view, domain::Stop*> stops, SphereProjector projector) const {
		std::vector<svg::Circle> result;
		for (const auto& [name, stop] : stops) {
			svg::Circle stop_point;
			stop_point.SetCenter(projector(stop->coordinates_));
			stop_point.SetRadius(render_settings_.stop_radius);
			stop_point.SetFillColor("white");
			result.push_back(stop_point);
		}
		return result;
	}

	std::vector<svg::Text> MapRenderer::VisualizeStopsNames(const std::map<std::string_view, domain::Stop*> stops, SphereProjector projector) const {
		std::vector<svg::Text> result;
		for (const auto& [name, stop] : stops) {
			svg::Text name;
			svg::Text name_u;

			name.SetPosition(projector(stop->coordinates_));
			name.SetOffset(render_settings_.stop_label_offset);
			name.SetFontSize(render_settings_.stop_label_font_size);
			name.SetFontFamily("Verdana");
			name.SetData(stop->stop_name_);
			name.SetFillColor("black");

			name_u.SetPosition(projector(stop->coordinates_));
			name_u.SetOffset(render_settings_.stop_label_offset);
			name_u.SetFontSize(render_settings_.stop_label_font_size);
			name_u.SetFontFamily("Verdana");
			name_u.SetData(stop->stop_name_);
			name_u.SetFillColor(render_settings_.underlayer_color);
			name_u.SetStrokeColor(render_settings_.underlayer_color);
			name_u.SetStrokeWidth(render_settings_.underlayer_width);
			name_u.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			name_u.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			result.push_back(name_u);
			result.push_back(name);
		}
		return result;
	}


	svg::Document MapRenderer::RenderBuses(const catalogue::TransportCatalogue& db) const {
		const std::vector<domain::Bus*> buses = db.GetBusesForMap();
		std::vector<geo::Coordinates> db_stop_coords;
		BusColor colors = DistributeColors(buses, render_settings_.color_palette);
		std::map<std::string_view, domain::Stop*> stops;
		for (auto& bus : buses) {
			for (auto& stop : bus->stops_) {
				db_stop_coords.push_back(stop->coordinates_);
				stops.insert({ stop->stop_name_, stop });
			}
		}
		SphereProjector projector(db_stop_coords.begin(), db_stop_coords.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
		
		svg::Document result;
		for (const auto& bus : buses) {
			auto line = VisualizeBusLines(bus, projector, colors);
			result.Add(line);			
		}
	
		for (const auto& bus : buses) {
			auto stops = VisualizeBusName(bus, projector, colors);
			for (size_t i = 0; i<stops.size(); i++) {
				result.Add(stops[i]);
			}
		}

		for (const auto& stop : VisualizeStopPoints(stops, projector)) {
			result.Add(stop);
		}
		for (const auto& stop : VisualizeStopsNames(stops, projector)) {
			result.Add(stop);
		}


		return result;
	}

	
}