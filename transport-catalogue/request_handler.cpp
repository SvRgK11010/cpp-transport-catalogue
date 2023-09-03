#include "request_handler.h"

namespace request_handler {

	using namespace catalogue;
	using namespace jsonreader;

	void RequestsProcessing(std::istream& input) {
		catalogue::TransportCatalogue catalogue1;

		jsonreader::JsonReader requests(input);

		requests.FillCatalogue(catalogue1);

		const auto& render_settings = requests.GetRenderSettings().AsDict();
		const auto& render = requests.SetRenderSettings(render_settings);
		const auto& stat_requests = requests.GetStatRequests();
		graph::DirectedWeightedGraph<double> tc_graph(catalogue1.GetAllStops().size());
		catalogue::RouteSettings settings = requests.SetRouteSettings(requests.GetRouteSettings().AsDict());
		catalogue::TransportRouter transport_router(catalogue1, settings);
		transport_router.BuildGraph(tc_graph);
		graph::Router<double> router(tc_graph);

		auto out = requests.ResponseToStatRequests(stat_requests, catalogue1, render, router, transport_router);

		json::Print(json::Document{ std::move(out)}, std::cout);


		//out.RenderMap().Render(std::cout);

	}
}