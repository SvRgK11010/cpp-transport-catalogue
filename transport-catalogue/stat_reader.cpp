#include "stat_reader.h"

using namespace std;
namespace Output {
	using namespace Catalogue;
	using namespace Geo;
	using namespace Input;

	void PrintInfo(TransportCatalogue& transport_catalogue) {
		int queries_count = ReadLineWithNumber();

		for (int i = 0; i < queries_count; i++) {
			std::string s = ReadLine();
			if (s[0] == 'B') {
				s.erase(0, 4); //удаление слова bus из запроса;

				if (transport_catalogue.GetBus(string_view{ s }) == nullptr) {
					std::cout << "Bus " << s << ": not found" << std::endl;
					continue;
				}
				BusInfo bus_info = transport_catalogue.GetBusInfo(string_view{ s });
				cout << "Bus " << bus_info.bus_numb_ << ": " <<
					bus_info.stop_count_ << " stops on route, " <<
					bus_info.unique_stops_ << " unique stops, " <<
					setprecision(6) <<
					bus_info.m_route_length_ << " route length, " <<
					bus_info.curvature << " curvature" << endl;
			}
			else if (s[0] == 'S') {
				s.erase(0, 5); //удаление слова stop из запроса;

				if (transport_catalogue.GetStop(string_view{ s }) == nullptr) {
					std::cout << "Stop " << s << ": not found" << std::endl;
					continue;
				}

				if (transport_catalogue.GetStopForBus(string_view{ s }) == nullptr) {
					cout << "Stop " << s << ": no buses" << endl;
					continue;
				}
				StopInfo stop_info = transport_catalogue.GetStopInfo(string_view{ s });
				cout << "Stop " << stop_info.stop_name_ << ": buses ";
				for (auto& bus : stop_info.buses_on_stop_) {
					cout << bus << " ";
				}
				cout << endl;

			}
		}

	}
}