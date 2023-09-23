#include "request_handler.h"

namespace request_handler {

	using namespace catalogue;
	using namespace jsonreader;

	void RequestsProcessingMakeBase(std::istream& input) {
		catalogue::TransportCatalogue catalogue1;

		jsonreader::JsonReader requests(std::cin);

		requests.FillCatalogue(catalogue1);

		const auto& settings = requests.GetRenderSettings().AsDict();
		map_renderer::MapRenderer renderer = requests.SetRenderSettings(settings);

		catalogue::RouteSettings r_settings = requests.SetRouteSettings(requests.GetRouteSettings().AsDict());
		catalogue::TransportRouter transport_router(catalogue1, r_settings);


		const auto& serialization_set = requests.GetSerializationSettings();

		serialization::Serialization serializator(catalogue1, renderer, transport_router);

		std::ofstream fout(serialization_set.AsDict().at("file").AsString(), std::ios::binary);

		if (fout.is_open()) {
			serializator.Serialize(fout);
		}
	}

	void RequestsProcessingMakeAnswers(std::istream& input) {
		catalogue::TransportCatalogue catalogue;
		map_renderer::RenderSettings settings;
		map_renderer::MapRenderer renderer(settings);
		catalogue::RouteSettings r_settings;
		catalogue::TransportRouter transport_router(catalogue, r_settings);


		jsonreader::JsonReader base_requests(std::cin);

		std::ifstream proto_db("transport_catalogue.db", std::ios::binary);

		serialization::Serialization deserializator(catalogue, renderer, transport_router);

		deserializator.Deserialize(proto_db);

		const auto& stat_requests = base_requests.GetStatRequests();
		
		graph::DirectedWeightedGraph<double> tc_graph(catalogue.GetAllStops().size());
		transport_router.BuildGraph(tc_graph);
		graph::Router<double> router(tc_graph);

		auto out = base_requests.ResponseToStatRequests(stat_requests, catalogue, renderer, router, transport_router);

		json::Print(json::Document{ std::move(out)}, std::cout);
	}

	
}