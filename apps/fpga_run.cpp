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
#include "table_data.hpp"

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

auto main() -> int {
  std::cout << "Starting main" << std::endl;
  auto data_type_sizes = DataManager::GetDataConfiguration("data_config.ini");
  TableData input_table =  DataManager::ParseDataFromCSV("MOCK_DATA.csv",
                                                     data_type_sizes);

  std::vector<int> column_sizes;
  int record_size = 0;
  for (auto column_type : input_table.table_column_label_vector) {
    column_sizes.push_back(column_type.second);
    record_size += column_type.second;
  }

  int record_count = input_table.table_data_vector.size() / record_size;
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

  for (int i = 0; i < input_table.table_data_vector.size(); i++) {
    input[i] = input_table.table_data_vector[i];
  }

  volatile uint32_t* input_data_phy = input_device->GetPhysicalAddress();
  volatile uint32_t* output_data_phy = output_device->GetPhysicalAddress();
  FPGAManager fpga_manager(&memory_manager);
  std::cout << "Main initialisation done!" << std::endl;

  fpga_manager.SetupQueryAcceleration(input_data_phy, output_data_phy,
                                      record_size, record_count);
  std::vector<int> result_sizes = fpga_manager.RunQueryAcceleration();
  std::cout << "Query done!" << std::endl;

  TableData resulting_table;
  resulting_table.table_data_vector =
      std::vector<uint32_t>(output, output + (result_sizes[0] * record_size));
  resulting_table.table_column_label_vector =
      input_table.table_column_label_vector;

  TableData expected_table =
      DataManager::ParseDataFromCSV("RESULT_DATA.csv", data_type_sizes);

  if (expected_table == resulting_table) {
    std::cout << "Query results are correct!" << std::endl;
  } else {
    std::cout << "Incorrect query results:" << std::endl;
    DataManager::PrintTableData(expected_table);
    std::cout << "vs:" << std::endl;
    DataManager::PrintTableData(resulting_table);
  }

  return 0;
}
