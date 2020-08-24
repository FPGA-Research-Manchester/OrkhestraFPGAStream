#include <iostream>
#include <string>
#include <vector>
#include <numeric>

#include "setup.hpp"
#include "data_manager.hpp"

/*
Filter: (price < 12000)
1000 rows
 Column |         Type          |
--------+-----------------------+
 id     | integer               |
 make   | character varying(32) |
 model  | character varying(32) |
 price  | integer               |
*/

auto main() -> int {
  //Data type sizes are currently hard coded
  std::vector<int> data_type_sizes{1, 8, 8, 1};
  int record_size =
      std::accumulate(data_type_sizes.begin(), data_type_sizes.end(), 0);

  std::vector<std::vector<std::string>> db_data;
  DataManager::AddStringDataFromCSV("MOCK_DATA.csv", db_data);

  std::vector<uint32_t> input_memory_area;
  DataManager::AddIntegerDataFromStringData(db_data, input_memory_area);

  std::vector<uint32_t> output_memory_area(input_memory_area.size());
  std::vector<uint32_t> module_configuration_memory_area((2 * 1024 * 1024), -1);
  Setup::SetupQueryAcceleration(
      module_configuration_memory_area.data(), input_memory_area.data(),
      output_memory_area.data(), record_size, db_data.size());

  //For testing
  DataManager::AddStringDataFromIntegerData(input_memory_area, db_data,
                                            data_type_sizes);
  DataManager::PrintStringData(db_data);

  return 0;
}