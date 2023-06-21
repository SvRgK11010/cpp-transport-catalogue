#include "input_reader.h"

using namespace std;

namespace Input {

    using namespace Catalogue;
    using namespace Geo;

    std::pair<std::string, std::string> SplitRequest(std::string& query) {
        pair<string, string> result;
        auto sp = query.find_first_not_of(' '); //начало запроса
        query.erase(0, sp + 5);        
        if (query.find(" to ") == string::npos) // в запросе есть только координаты, расстояний нет
        {
            result.first = move(query);
            result.second = "";
        }
        else if (query.find(" to ") != string::npos) //после координат есть дистанции до др остановок
        {
            string name;
            sp = query.find_first_not_of(' '); //начало наим. остановки
            auto delim = query.find(':'); // Окончание наименования остановки
            name = query.substr(sp, delim + 1);
            auto comma = query.find(',');
            string first_q = query.substr(sp, comma + 1);
            query.erase(0, comma + 1);
            comma = query.find(',');
            first_q += query.substr(0, comma);
            query.erase(0, comma + 1);
            string second_q = name + query;
            result = { move(first_q), move(second_q) };

        }
        return result;
    }

    Stop ParseStop(std::string& query) {
        string name;
        auto delim = query.find(':'); // Окончание наименования остановки
        name = query.substr(0, delim);
        query.erase(0, delim + 1);
        auto sp = query.find_first_not_of(' '); //найти первый не пробел = начало первой координаты
        auto comma = query.find(','); //запятая
        string lat = query.substr(sp, comma - 1);
        query.erase(0, comma + 1);
        sp = query.find_first_not_of(' ');
        string lng = query.substr(sp, query.back());
        if (query.find(',') != string::npos) { //после координат есть дистанции
            comma = query.find(',');
            lng = query.substr(0, comma - 1);
            query.erase(0, comma + 1);
        }
        double lngs = stod(lng);
        double lats = stod(lat);
        Coordinates coord = { lats, lngs };
        Stop stop(name, coord);
        return stop;
    }



    std::unordered_map<std::pair<Stop*, Stop*>, size_t, StopHasher> ParseDistances(std::string& query, TransportCatalogue& transport_cat) {
        unordered_map<std::pair<Stop*, Stop*>, size_t, StopHasher> result;
        result.reserve(100);
        auto delim = query.find(':'); // Окончание наименования остановки
        string name = query.substr(0, delim);
        Stop* x_ = transport_cat.GetStop(name);
        query.erase(0, delim + 2);//удаление имени остановки
        while (!query.empty()) {
            auto m_ = query.find("m ");
            string meter = query.substr(0, m_);
            size_t met = stoi(meter);
            query.erase(0, query.find("to ") + 3);
            if (query.find(',') == string::npos) //последняя остановка в запросе 
            {
                string sec_name = query.substr(0, query.back());
                auto y_ = transport_cat.GetStop(sec_name);
                result.insert({ {x_, y_}, met });
                break;
            }
            delim = query.find(',');
            string sec_name = query.substr(0, delim);
            Stop* y_ = transport_cat.GetStop(sec_name);
            result.insert({ {x_, y_}, met });
            query.erase(0, delim + 2);
        }
        return result;
    }

    Bus ParseBus(std::string& query, TransportCatalogue& transport_cat) {
        Bus bus;
        query.erase(0, 4); //удаляем слово bus и пробел после него
        auto delim = query.find(':');
        bus.bus_number_ = query.substr(0, delim);
        query.erase(0, delim + 1);
        string stopname;
        vector <Stop*> stops;
        if (query.find('>') != string::npos) { //кольцевой маршрут
            auto ring = query.find('>');
            while (!query.empty()) {
                stopname = query.substr(query.find_first_not_of(' '), ring - 2);
                bus.stops_.push_back(transport_cat.GetStop(string_view{ stopname }));
                stopname.clear();
                query.erase(0, ring + 1);
                if (query.find('>') == string::npos) {//последняя остановка в запросе
                    stopname = query.substr(query.find_first_not_of(' '), query.find_last_not_of(' '));
                    bus.stops_.push_back(transport_cat.GetStop(string_view{ stopname }));
                    stopname.clear();
                    break;
                }
                ring = query.find('>');
            }
        }
        else if (query.find('-') != string::npos) //стандартный маршрут
        {
            auto standart = query.find('-');
            while (!query.empty()) {
                stopname = query.substr(query.find_first_not_of(' '), standart - 2);
                bus.stops_.push_back(transport_cat.GetStop(string_view{ stopname }));
                stopname.clear();
                query.erase(0, standart + 1);

                if (query.find('-') == string::npos) {//последняя остановка в запросе
                    stopname = query.substr(query.find_first_not_of(' '), query.find_last_not_of(' '));
                    bus.stops_.push_back(transport_cat.GetStop(string_view{ stopname }));
                    stopname.clear();
                    break;
                }
                standart = query.find('-');
            }
            auto stops_standart = bus.stops_;
            stops_standart.pop_back();
            reverse(stops_standart.begin(), stops_standart.end());
            for (auto i = 0; i < stops_standart.size(); i++) {
                bus.stops_.push_back(stops_standart[i]);
            }
        }
        return bus;
    }

    TransportCatalogue CreateTransportCatalogue(std::istream& input) {
        TransportCatalogue transport_cat;
        vector<string> buses_queries;
        vector<pair<string, string>> stops_queries;
        int queries_count;
        input >> queries_count;

        //Накопление запросов по видам: остановки, маршруты
        for (int i = 0; i < queries_count; i++) {
            string query;
            getline(input, query);
            if (query[0] == 'B') {
                buses_queries.push_back(move(query));
            }
            else if (query[0] == 'S') {
                stops_queries.push_back(SplitRequest(query));
                Stop stop = ParseStop(stops_queries.back().first);
                transport_cat.AddStop(move(stop));
            }
            query.clear();
        }
        
        //Добавление дистанций в каталог
        for (auto& stop : stops_queries) {
            if (stop.second.empty()) { continue; }
            auto dist = ParseDistances(stop.second, transport_cat);
            for (auto& [key, val] : dist) {
                transport_cat.SetDistances(key.first, key.second, val);
            }
        }
        stops_queries.clear();
        //Добавление маршрутов в каталог
        for (auto& bus : buses_queries) {
            Bus buss = ParseBus(bus, transport_cat);
            transport_cat.AddBus(move(buss));
        }
        buses_queries.clear();
    }
}