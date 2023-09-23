#pragma once
#include "json.h"
#include "transport_catalogue.pb.h"
#include "svg.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.pb.h"

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "graph.h"

#include <algorithm>
#include <utility>
#include <variant>

namespace serialization {

	class Serialization {
	public:

		Serialization(catalogue::TransportCatalogue& db, map_renderer::MapRenderer& renderer, catalogue::TransportRouter& router)
			: db_(db), renderer_(renderer), router_(router) {}

		void Serialize(std::ostream& out);
		void Deserialize(std::istream& input);

	
	private:
		catalogue::TransportCatalogue& db_;
		map_renderer::MapRenderer& renderer_;
		catalogue::TransportRouter& router_;
		proto_catalogue::TransportCatalogue proto_db_;

		void SerializeStops();
		void SerializeBuses();
		void SerializeDistances();

		proto_map_renderer::Color SerializeColor(const svg::Color& color);
		proto_map_renderer::Point SerializePoint(const svg::Point& point);
		void SerializeRenderSettings();

		void SerializeRouter();
		proto_transport_router::RouteSettings SerializeRouterSettings();

		void DeserializeStops();
		void DeserializeBuses();
		void DeserializeDistances();

		svg::Color DeserializeColor(const proto_map_renderer::Color& p_color);
		svg::Point DeserializePoint(const proto_map_renderer::Point& p_point);
		void DeserializeRenderSettings();

		void DeserializeRouter();		
		catalogue::RouteSettings DeserializeRouterSettings();
	};
	


}
