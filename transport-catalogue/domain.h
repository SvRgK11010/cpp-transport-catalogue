#pragma once

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
#include "geo.h"

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace domain {
	using namespace geo;

	struct Stop {
		std::string stop_name_;
		Coordinates coordinates_;
		std::set<std::string_view> buses_on_stop_;

		explicit Stop(std::string name, Coordinates coord)
			: stop_name_(name), coordinates_(coord)
		{
		}
	};

	struct Bus {
		explicit Bus(std::string number, std::vector<Stop*> stops, bool roundtrip) 
			: bus_number_(number), stops_(stops), roundtrip_(roundtrip) 
		{
		}

		std::string bus_number_;
		std::vector<Stop*> stops_ = {};
		bool roundtrip_;
	};

	struct BusInfo {
		std::string_view bus_numb_;
		int stop_count_ = 0;
		int unique_stops_ = 0;
		double geo_route_length_ = 0.0;
		int m_route_length_ = 0;
		double curvature = 0.0;
	};
}