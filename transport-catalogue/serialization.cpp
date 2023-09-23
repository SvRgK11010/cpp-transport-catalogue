#include "serialization.h"

namespace serialization {
	void Serialization::Serialize(std::ostream& out) {
		SerializeStops();
		SerializeDistances();
		SerializeBuses();
		SerializeRenderSettings();
		SerializeRouter();

		proto_db_.SerializeToOstream(&out);
	}

	void Serialization::Deserialize(std::istream& input) {
		proto_db_.ParseFromIstream(&input);

		DeserializeStops();
		DeserializeDistances();
		DeserializeBuses();
		DeserializeRenderSettings();
		DeserializeRouter();
	}

	

	void Serialization::SerializeStops() {
		const auto stops = db_.GetAllStops();
		for (const auto& stop : stops) {
			proto_catalogue::Stop p_stop;
			p_stop.set_stop_name(stop.stop_name_);
			p_stop.mutable_coordinates()->set_lat(stop.coordinates_.lat);
			p_stop.mutable_coordinates()->set_lng(stop.coordinates_.lng);
			for (const auto& bus : stop.buses_on_stop_) {
				p_stop.add_buses_on_stop(std::string{ bus });
			 }
			*proto_db_.add_stops() = (std::move(p_stop));
		}
	}
	void Serialization::SerializeBuses() {
		const auto buses = db_.GetAllBuses();
		for (const auto& bus : buses) {
			proto_catalogue::Bus p_bus;
			p_bus.set_bus_number(bus.bus_number_);
			p_bus.set_roundtrip(bus.roundtrip_);
			for (const auto& stop : bus.stops_) {
				*p_bus.mutable_stops()->Add() = stop->stop_name_;
			}
			*proto_db_.add_buses() = std::move(p_bus);
		}
	}
	void Serialization::SerializeDistances() {
		for (const auto& [from_to, dist] : db_.GetDistances()) {
			proto_catalogue::Distances p_dist;
			p_dist.set_from(from_to.first->stop_name_);
			p_dist.set_to(from_to.second->stop_name_);
			p_dist.set_distance(dist);

			*proto_db_.add_stop_distances() = std::move(p_dist);
	}
	}

	proto_map_renderer::Color Serialization::SerializeColor(const svg::Color& color) {
		proto_map_renderer::Color p_color;
		if (std::holds_alternative<std::monostate>(color)) {
			p_color.set_nonecolor(true);
			return p_color;
		}
		else if (std::holds_alternative<svg::Rgb>(color)) {
			proto_map_renderer::Rgb p_rgb;
			svg::Rgb rgb = std::get<svg::Rgb>(color);
			p_rgb.set_red(rgb.red);
			p_rgb.set_green(rgb.green);
			p_rgb.set_blue(rgb.blue);
			*p_color.mutable_rgb() = p_rgb;
			return p_color;
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			proto_map_renderer::Rgba p_rgba;
			svg::Rgba rgba = std::get<svg::Rgba>(color);
			p_rgba.set_red(rgba.red);
			p_rgba.set_green(rgba.green);
			p_rgba.set_blue(rgba.blue);
			p_rgba.set_opacity(rgba.opacity);
			*p_color.mutable_rgba() = p_rgba;
			return p_color;
		}
		else {
			std::string color_name = std::get<std::string>(color);
			p_color.set_name(color_name);
			return p_color;
		}


	}
	proto_map_renderer::Point Serialization::SerializePoint(const svg::Point& point) {
		proto_map_renderer::Point p_point;
		p_point.set_x(point.x);
		p_point.set_y(point.y);
		return p_point;

	}
	void Serialization::SerializeRenderSettings() {
		const auto render_settings = renderer_.GetRenderSettings();
		proto_map_renderer::RenderSettings p_render_settings;
		p_render_settings.set_width(render_settings.width);
		p_render_settings.set_height(render_settings.height);
		p_render_settings.set_padding(render_settings.padding);
		p_render_settings.set_stop_radius(render_settings.stop_radius);
		p_render_settings.set_underlayer_width(render_settings.underlayer_width);
		p_render_settings.set_line_width(render_settings.line_width);
		p_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
		p_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
		*p_render_settings.mutable_bus_label_offset() = SerializePoint(render_settings.bus_label_offset);
		*p_render_settings.mutable_stop_label_offset() = SerializePoint(render_settings.stop_label_offset);
		*p_render_settings.mutable_underlayer_color() = SerializeColor(render_settings.underlayer_color);
		for (const auto& color : render_settings.color_palette) {
			p_render_settings.mutable_color_palette()->Add(SerializeColor(color));
			//std::cout << "color_palette_size: " << p_render_settings.color_palette_size() << std::endl;
		}

		*proto_db_.mutable_render_settings() = std::move(p_render_settings);
	}

	void Serialization::SerializeRouter() {
		proto_transport_router::TransportRouter p_router;
		*p_router.mutable_settings() = SerializeRouterSettings();
		*proto_db_.mutable_router() = std::move(p_router);
	}
	
	proto_transport_router::RouteSettings Serialization::SerializeRouterSettings() {
		proto_transport_router::RouteSettings p_settings;
		const auto settings = router_.GetRouteSettings();
		p_settings.set_bus_velocity(settings.bus_velocity_);
		p_settings.set_bus_wait_time(settings.bus_wait_time_);
		return p_settings;
	}

	void Serialization::DeserializeStops() {
		for (size_t i = 0; i < proto_db_.stops_size(); i++) {
			const proto_catalogue::Stop& stop = proto_db_.stops(i);
			db_.AddStop(stop.stop_name(), { stop.coordinates().lat(), stop.coordinates().lng() });
		}
	
	}
	void Serialization::DeserializeBuses() {
		for (size_t i = 0; i < proto_db_.buses_size(); i++) {
			const proto_catalogue::Bus& bus = proto_db_.buses(i);
			std::vector<domain::Stop*> stops(bus.stops_size());
			for (size_t j = 0; j < bus.stops_size(); j++) {
				stops[j] = db_.GetStop(bus.stops(j));
			}
			db_.AddBus(bus.bus_number(), stops, bus.roundtrip());
		}
	
	}
	void Serialization::DeserializeDistances() {
		for (size_t i = 0; i < proto_db_.stop_distances_size(); i++) {
			const proto_catalogue::Distances& dist = proto_db_.stop_distances(i);
			domain::Stop* from = db_.GetStop(dist.from());
			domain::Stop* to = db_.GetStop(dist.to());
			db_.FillDistances(from, to, dist.distance());
		}
	
	}

	svg::Color Serialization::DeserializeColor(const proto_map_renderer::Color& p_color) {
		if (p_color.has_rgb()) {
			return svg::Color{svg::Rgb{static_cast<uint8_t>(p_color.rgb().red()),
				static_cast<uint8_t>(p_color.rgb().green()),
				static_cast<uint8_t>(p_color.rgb().blue()), }};
		}
		else if (p_color.has_rgba()) {
			return svg::Color{svg::Rgba{static_cast<uint8_t>(p_color.rgba().red()),
				static_cast<uint8_t>(p_color.rgba().green()),
				static_cast<uint8_t>(p_color.rgba().blue()), 
				p_color.rgba().opacity(), }};
		}
		else if (p_color.nonecolor()) {
			return std::monostate();
		}
		else {
			return p_color.name();
		}
	}

	svg::Point Serialization::DeserializePoint(const proto_map_renderer::Point& p_point) {
		return { p_point.x(), p_point.y() };
	}

	void Serialization::DeserializeRenderSettings() {
		map_renderer::RenderSettings settings;
		const proto_map_renderer::RenderSettings p_settings = proto_db_.render_settings();
		settings.width = p_settings.width();
		settings.height = p_settings.height();
		settings.padding = p_settings.padding();
		settings.stop_radius = p_settings.stop_radius();
		settings.line_width = p_settings.line_width();
		settings.bus_label_font_size = p_settings.bus_label_font_size();
		settings.stop_label_font_size = p_settings.stop_label_font_size();
		settings.underlayer_width = p_settings.underlayer_width();
		settings.bus_label_offset = DeserializePoint(p_settings.bus_label_offset());
		settings.stop_label_offset = DeserializePoint(p_settings.stop_label_offset());
		settings.underlayer_color = DeserializeColor(p_settings.underlayer_color());
		for (int i = 0; i < p_settings.color_palette_size(); i++) {
			settings.color_palette.emplace_back(DeserializeColor(p_settings.color_palette(i)));
		}
		renderer_.SetRenderSettings(settings);
	}

	void Serialization::DeserializeRouter() {
		router_.SetRouteSettings(DeserializeRouterSettings());
	}
	
	catalogue::RouteSettings Serialization::DeserializeRouterSettings() {
		const proto_transport_router::RouteSettings& p_settings = proto_db_.router().settings();
		catalogue::RouteSettings settings;
		settings.bus_velocity_ = p_settings.bus_velocity();
		settings.bus_wait_time_ = p_settings.bus_wait_time();
		return settings;
	}
}