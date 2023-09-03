#include "transport_router.h"

namespace catalogue {
	void TransportRouter::BuildGraph(graph::DirectedWeightedGraph<double>& graph) {
		for (const auto& bus : tc_.GetAllBuses()) {
				const auto& stops = bus.stops_;
				double weight = rs_.bus_wait_time_ * 1.0;
				size_t span = 0;
				if (stops.size() > 1) {
					for (size_t i = 0; i < stops.size(); i++) {
						span = 1;
						weight = rs_.bus_wait_time_ * 1.0;
						for (size_t j = i + 1; j < stops.size(); j++) {
							if (stops[i] != stops[j]) {
								auto dist = tc_.GetDistance(stops[j - 1], stops[j]);
								
								weight += dist*1.0 / 1000*1.0 / rs_.bus_velocity_ * 60*1.0;
								//std::cout << "1edge weight: " << weight << std::endl;
								graph.AddEdge({ stops[i]->id_, stops[j]->id_, weight, span, bus.bus_number_ });
								++span;
							}
						}
					}
					if (!bus.roundtrip_) {
						for (size_t i = stops.size() - 1u; i > 0; --i) {
							span = 1;
							weight = rs_.bus_wait_time_ * 1.0;
							for (size_t j = i; j > 0; --j) {
								if (stops[i] != stops[j - 1]) {
									auto dist = tc_.GetDistance(stops[j], stops[j - 1]);
					
									weight += dist * 1.0 / 1000 * 1.0 / rs_.bus_velocity_ * 60 * 1.0;
									//std::cout << "edge weight: " << weight << std::endl;		
									graph.AddEdge({ stops[i]->id_, stops[j - 1]->id_, weight, span, bus.bus_number_ });
									++span;
								}
							}
						}
					}
				}
			}
	}

	const RouteSettings TransportRouter::GetRouteSettings() const {
		return rs_;
	}
}