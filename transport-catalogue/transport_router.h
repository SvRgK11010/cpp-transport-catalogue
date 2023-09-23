#pragma once
#include <iostream>
#include <string>

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


		void SetRouteSettings(const RouteSettings& settings);
		void AddEdgeToGraph(Stop* from, Stop* to, Stop* from_dist, double& weight, size_t& span, const std::string& bus, graph::DirectedWeightedGraph<double>& graph);
		void BuildGraph(graph::DirectedWeightedGraph<double>& graph);
		std::optional<graph::Router<double>::RouteInfo> FindRoute(Stop* from, Stop* to, const graph::Router<double>& router) const;
		const RouteSettings GetRouteSettings() const;

	private:
		const catalogue::TransportCatalogue& tc_;
		RouteSettings rs_;
		
	};
}