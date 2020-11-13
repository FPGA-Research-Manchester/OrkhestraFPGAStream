#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

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

auto GetRecordSize(TableData& input_table) -> int {
  int record_size = 0;
  for (auto column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void CheckTableData(TableData& expected_table, TableData& resulting_table) {
  if (expected_table == resulting_table) {
    std::cout << "Query results are correct!" << std::endl;
  } else {
    std::cout << "Incorrect query results:" << std::endl;
    DataManager::PrintTableData(expected_table);
    std::cout << "vs:" << std::endl;
    DataManager::PrintTableData(resulting_table);
  }
}

void ReadOutputData(std::unique_ptr<MemoryBlockInterface>& output_device,
                    TableData& resulting_table, int& result_size) {
  volatile uint32_t* output = output_device->GetVirtualAddress();
  resulting_table.table_data_vector = std::vector<uint32_t>(
      output, output + (result_size * GetRecordSize(resulting_table)));
}

void WriteInputData(std::unique_ptr<MemoryBlockInterface>& input_device,
                    TableData& input_table) {
  volatile uint32_t* input = input_device->GetVirtualAddress();
  for (int i = 0; i < input_table.table_data_vector.size(); i++) {
    input[i] = input_table.table_data_vector[i];
  }
}

void RunQueryWithData(DataManager& data_manager, FPGAManager& fpga_manager,
                      std::unique_ptr<MemoryBlockInterface>& input_device,
                      std::unique_ptr<MemoryBlockInterface>& output_device,
                      std::string& input_data_filename,
                      std::string& expected_data_filename) {
  auto input_table = data_manager.ParseDataFromCSV(input_data_filename);
  WriteInputData(input_device, input_table);

  fpga_manager.SetupQueryAcceleration(
      input_device->GetPhysicalAddress(), output_device->GetPhysicalAddress(),
      GetRecordSize(input_table),
      input_table.table_data_vector.size() / GetRecordSize(input_table));

  std::cout << "Running query!" << std::endl;
  auto result_sizes = fpga_manager.RunQueryAcceleration();
  std::cout << "Query done!" << std::endl;

  TableData resulting_table;
  resulting_table.table_column_label_vector =
      input_table.table_column_label_vector;
  ReadOutputData(output_device, resulting_table, result_sizes[0]);

  auto expected_table = data_manager.ParseDataFromCSV(expected_data_filename);
  CheckTableData(expected_table, resulting_table);
}

auto main() -> int {
  std::cout << "Starting up!" << std::endl;
  DataManager data_manager("data_config.ini");
  MemoryManager memory_manager("DSPI_filtering");
  FPGAManager fpga_manager(&memory_manager);
  auto input_device = memory_manager.AllocateMemoryBlock();
  auto output_device = memory_manager.AllocateMemoryBlock();

  std::string input_data_filename = "CUSTOMER_DATA.csv";
  std::string expected_data_filename = "CUSTOMER_DATA.csv";

  RunQueryWithData(data_manager, fpga_manager, input_device, output_device,
                   input_data_filename, expected_data_filename);

  input_data_filename = "CAR_DATA.csv";
  expected_data_filename = "CAR_DATA.csv";
  //expected_data_filename = "CAR_FILTER_DATA.csv";

  RunQueryWithData(data_manager, fpga_manager, input_device, output_device,
                   input_data_filename, expected_data_filename);

  return 0;
}
