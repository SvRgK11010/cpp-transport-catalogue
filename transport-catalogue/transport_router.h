#pragma once
#include <iostream>

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "domain.h"

namespace catalogue {
	struct RouteSettings {
		int bus_wait_time_ = 0;
		double bus_velocity_ = 0.0;
	};
	class TransportRouter {
	public:
		TransportRouter(const catalogue::TransportCatalogue& catalogue, RouteSettings settings)
			:tc_(catalogue), rs_(settings) {}

		void BuildGraph(graph::DirectedWeightedGraph<double>& graph);
		const RouteSettings GetRouteSettings() const;

	private:
		const catalogue::TransportCatalogue& tc_;
		RouteSettings rs_;
		graph::Router<double>* router_ = nullptr;
	};
}