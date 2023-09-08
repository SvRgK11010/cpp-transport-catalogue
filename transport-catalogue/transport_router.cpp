#include "transport_router.h"

namespace catalogue {

	void TransportRouter::AddEdgeToGraph(Stop* from, Stop* to, Stop* from_dist, double& weight, size_t& span, const std::string& bus, graph::DirectedWeightedGraph<double>& graph) {
		double MET_TO_KM = 1000.0;
		double MIN_TO_HOUR = 60.0;
		auto distance = static_cast<double>(tc_.GetDistance(from_dist, to));
		weight += distance / MET_TO_KM / rs_.bus_velocity_ * MIN_TO_HOUR;
		graph.AddEdge({ from->id_, to->id_, weight, span, bus });

	}

	void TransportRouter::BuildGraph(graph::DirectedWeightedGraph<double>& graph) {
		for (const auto& bus : tc_.GetAllBuses()) {
			const auto& stops = bus.stops_;
			double weight = static_cast<double>(rs_.bus_wait_time_);
			size_t span = 0;
			if (stops.size() > 1) {
				for (size_t i = 0; i < stops.size(); i++) {
					span = 1;
					weight = static_cast<double>(rs_.bus_wait_time_);
					for (size_t j = i + 1; j < stops.size(); j++) {
						if (stops[i] != stops[j]) {

							AddEdgeToGraph(stops[i], stops[j], stops[j - 1], weight, span, bus.bus_number_, graph);

							++span;
						}
					}
				}
				if (!bus.roundtrip_) {
					for (size_t i = stops.size() - 1u; i > 0; --i) {
						span = 1;
						weight = static_cast<double>(rs_.bus_wait_time_);
						for (size_t j = i; j > 0; --j) {
							if (stops[i] != stops[j - 1]) {
								AddEdgeToGraph(stops[i], stops[j - 1], stops[j], weight, span, bus.bus_number_, graph);
								++span;
							}
						}
					}
				}
			}
		}
	}

	std::optional<graph::Router<double>::RouteInfo> TransportRouter::FindRoute(Stop* from, Stop* to, const graph::Router<double>& router) const {
		return router.BuildRoute(from->id_, to->id_);
	}

	const RouteSettings TransportRouter::GetRouteSettings() const {
		return rs_;
	}
}