#pragma once
#include <optional>
#include <sstream>
#include "transport_catalogue.h"
#include "json_reader.h"
#include "json.h"
#include "map_renderer.h"
#include "svg.h"

class RequestHandler {    
    
public:

    explicit RequestHandler(const catalogue::TransportCatalogue& db, const map_renderer::MapRenderer& renderer)
        : db_(db), renderer_(renderer)
    {
    }

    void RequestsProcessing(const json::Node& all_requests) const;

    const json::Node FillBusInfo(const json::Dict& bus_info) const;
    const json::Node FillStopInfo(const json::Dict& stop_info) const;
    const json::Node FillMapInfo(const json::Dict& map_info) const;

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<domain::BusInfo> GetBusInfo(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через (запрос Stop)
    const std::set<std::string_view> GetStopInfo(const std::string_view& stop_name) const;

    
    svg::Document RenderMap() const;

private:
    const catalogue::TransportCatalogue& db_;
    const map_renderer::MapRenderer& renderer_;
};
