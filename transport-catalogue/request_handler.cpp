#include "request_handler.h"

namespace request_handler {

	using namespace catalogue;
	using namespace jsonreader;

	void RequestsProcessing(std::istream& input) {
		catalogue::TransportCatalogue catalogue1;

		jsonreader::JsonReader requests(input);

		requests.FillCatalogue(catalogue1);

		const auto& render_settings = requests.GetRenderSettings().AsMap();
		const auto& render = requests.SetRenderSettings(render_settings);
		const auto& stat_requests = requests.GetStatRequests();
		
		auto out = requests.ResponseToStatRequests(stat_requests, catalogue1, render);

		json::Print(json::Document{ std::move(out)}, std::cout);


		//out.RenderMap().Render(std::cout);

	}
}