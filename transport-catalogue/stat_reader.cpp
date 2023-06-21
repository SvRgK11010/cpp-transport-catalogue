#include "stat_reader.h"

using namespace std;
namespace Output {
	using namespace Catalogue;
	using namespace Geo;
	using namespace Input;

	void PrintStopInfo(const StopInfo& stop) {
		cout << "Stop " << stop.stop_name_ << ": buses ";
		for (auto& bus : stop.buses_on_stop_) {
			cout << bus << " ";
		}
		cout << endl;
	}

	void PrintBusInfo(const BusInfo& bus) {
		cout << "Bus " << bus.bus_numb_ << ": " <<
			bus.stop_count_ << " stops on route, " <<
			bus.unique_stops_ << " unique stops, " <<
			setprecision(6) <<
			bus.m_route_length_ << " route length, " <<
			bus.curvature << " curvature" << endl;
	}

	void ReadStat(istream& input, TransportCatalogue& transport_catalogue, ostream& output) {
		int queries_count;
		input >> queries_count;

		for (int i = 0; i < queries_count; i++) {
			std::string s;
			getline(input, s);
			if (s[0] == 'B') {
				s.erase(0, 4); //удаление слова bus из запроса;

				if (transport_catalogue.GetBus(string_view{ s }) == nullptr) {
					output << "Bus " << s << ": not found" << std::endl;
					continue;
				}
				BusInfo bus_info = transport_catalogue.GetBusInfo(string_view{ s });
				PrintBusInfo(bus_info);
			}
			else if (s[0] == 'S') {
				s.erase(0, 5); //удаление слова stop из запроса;

				if (transport_catalogue.GetStop(string_view{ s }) == nullptr) {
					output << "Stop " << s << ": not found" << std::endl;
					continue;
				}

				if (transport_catalogue.GetStopForBus(string_view{ s }) == nullptr) {
					output << "Stop " << s << ": no buses" << endl;
					continue;
				}
				StopInfo stop_info = transport_catalogue.GetStopInfo(string_view{ s });
				PrintStopInfo(stop_info);

			}
		}

	}
}