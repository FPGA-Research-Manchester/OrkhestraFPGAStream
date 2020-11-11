#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include "data_manager.hpp"
#include "fpga_manager.hpp"
#include "memory_block_interface.hpp"
#include "memory_manager.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

/*
Filter: (price < 12000)
1000 rows
 Column |         Type          |
--------+-----------------------+
 id     | bigint                |
 make   | character varying(32) |
 model  | character varying(32) |
 price  | integer               |
*/

auto manual_aligned_alloc(uint8_t alignment, uint64_t size) -> void* {
  auto potentially_unaligned_buffer = (uint64_t)malloc(size + alignment);
  uint64_t aligned_buffer =
      ((potentially_unaligned_buffer + alignment) / alignment) * alignment;
  auto* buffer = (uint8_t*)aligned_buffer;
  *(buffer - 1) = aligned_buffer - potentially_unaligned_buffer;
  return buffer;
}

void manual_aligned_free(void* ptr) {
  if (ptr) {
    auto* buffer = static_cast<uint8_t*>(ptr);
    uint8_t shift = *(buffer - 1);
    buffer -= shift;
    free(buffer);
  }
}


// Create config - In the config you have data type sizes written down - int = 1 char = 1/4
// Then read first line of CSV Int-1,Char-32,Char-32,Int-1
// Then you get column_sizes and data_types + existing data = Create object with that which can return sizes for FPGA_Manager and types for DataManager itself.



// DataManager - Read config - creates data config map
// DataManager - ParseCSV - Creates data table struct
// Struct can be used to put data to DDR and into the FPGA setup methods
// Output struct can be created as well.
// Two structs can be compared :)


auto main() -> int {
  std::cout << "Starting main" << std::endl;
  auto data_type_sizes = DataManager::GetDataConfiguration("data_config.ini");
  std::map<std::string, int> table_column_types;
  table_column_types.insert(std::pair<std::string, int>("integer", 1));
  table_column_types.insert(std::pair<std::string, int>("varchar", 32));
  table_column_types.insert(std::pair<std::string, int>("varchar", 32));
  table_column_types.insert(std::pair<std::string, int>("integer", 1));

  std::vector<std::string> table_column_vector = {"integer", "varchar",
                                                  "varchar", "integer"};
 
  std::vector<int> column_sizes;
  for (auto column_type : table_column_vector) {
    column_sizes.push_back(data_type_sizes[column_type] *
                           table_column_types[column_type]);
  }

  int record_size = 0;
  for (int row_size : column_sizes) {
    record_size += row_size;
  }

  std::vector<std::vector<std::string>> db_data;
  DataManager::AddStringDataFromCSV("MOCK_DATA.csv", db_data);

  std::vector<uint32_t> input_memory_area;
  DataManager::AddIntegerDataFromStringData(db_data, input_memory_area);
  db_data.clear();

  int record_count = input_memory_area.size() / record_size;
  int output_memory_size =
      (record_count +
       record_count %
           StreamParameterCalculator::FindMinViableRecordsPerDDRBurst(
               query_acceleration_constants::kDdrBurstSize, record_size)) *
      record_size;

  MemoryManager memory_manager = MemoryManager("DSPI_filtering");

  std::unique_ptr<MemoryBlockInterface> input_device =
      memory_manager.AllocateMemoryBlock();
  std::unique_ptr<MemoryBlockInterface> output_device =
      memory_manager.AllocateMemoryBlock();

  volatile uint32_t* input = input_device->GetVirtualAddress();
  volatile uint32_t* output = output_device->GetVirtualAddress();

  for (int i = 0; i < input_memory_area.size(); i++) {
    input[i] = input_memory_area[i];
  }

  volatile uint32_t* input_data_phy = input_device->GetPhysicalAddress();
  volatile uint32_t* output_data_phy = output_device->GetPhysicalAddress();
  FPGAManager fpga_manager(&memory_manager);
  std::cout << "Main initialisation done!" << std::endl;

  fpga_manager.SetupQueryAcceleration(input_data_phy, output_data_phy,
                                      record_size, record_count);
  std::vector<int> result_sizes = fpga_manager.RunQueryAcceleration();
  std::cout << "Query done!" << std::endl;
  DataManager::AddStringDataFromIntegerData(
      std::vector<uint32_t>(output, output + (result_sizes[0] * record_size)),
      db_data, column_sizes);

  std::vector<std::vector<std::string>> golden_data;
  DataManager::AddStringDataFromCSV("RESULT_DATA.csv", golden_data);

  if (golden_data == db_data) {
    std::cout << "Query results are correct!" << std::endl;
  } else {
    std::cout << "Incorrect query results:" << std::endl;
    DataManager::PrintStringData(db_data);
    std::cout << "vs:" << std::endl;
    DataManager::PrintStringData(golden_data);
  }

  return 0;
}
