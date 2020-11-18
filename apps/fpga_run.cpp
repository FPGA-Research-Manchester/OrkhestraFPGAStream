#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
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

// auto manual_aligned_alloc(uint8_t alignment, uint64_t size) -> void* {
//  auto potentially_unaligned_buffer = (uint64_t)malloc(size + alignment);
//  uint64_t aligned_buffer =
//      ((potentially_unaligned_buffer + alignment) / alignment) * alignment;
//  auto* buffer = (uint8_t*)aligned_buffer;
//  *(buffer - 1) = aligned_buffer - potentially_unaligned_buffer;
//  return buffer;
//}
//
// void manual_aligned_free(void* ptr) {
//  if (ptr) {
//    auto* buffer = static_cast<uint8_t*>(ptr);
//    uint8_t shift = *(buffer - 1);
//    buffer -= shift;
//    free(buffer);
//  }
//}

auto GetRecordSize(TableData& input_table) -> int {
  int record_size = 0;
  for (const auto& column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void CheckTableData(TableData& expected_table, TableData& resulting_table) {
  if (expected_table == resulting_table) {
    std::cout << "Query results are correct!" << std::endl;
  } else {
    std::cout << "Incorrect query results:" << std::endl;
    /*std::cout << expected_table.table_data_vector.size() /
                     GetRecordSize(expected_table)
              << std::endl;*/
    DataManager::PrintTableData(expected_table);
    std::cout << "vs:" << std::endl;
    /*std::cout << resulting_table.table_data_vector.size() /
                     GetRecordSize(resulting_table)
              << std::endl;*/
    DataManager::PrintTableData(resulting_table);
  }
}

void ReadOutputData(const std::unique_ptr<MemoryBlockInterface>& output_device,
                    TableData& resulting_table, int& result_size) {
  volatile uint32_t* output = output_device->GetVirtualAddress();
  resulting_table.table_data_vector = std::vector<uint32_t>(
      output, output + (result_size * GetRecordSize(resulting_table)));
}

void WriteInputData(const std::unique_ptr<MemoryBlockInterface>& input_device,
                    TableData& input_table) {
  volatile uint32_t* input = input_device->GetVirtualAddress();
  for (int i = 0; i < input_table.table_data_vector.size(); i++) {
    input[i] = input_table.table_data_vector[i];
  }
}

void RunQueryWithData(
    DataManager& data_manager, FPGAManager& fpga_manager,
    const std::vector<std::pair<std::unique_ptr<MemoryBlockInterface>,
                                std::string>>& input_data_locations,
    const std::vector<std::pair<std::unique_ptr<MemoryBlockInterface>,
                                std::string>>& output_data_locations) {
  std::vector<StreamInitialisationData> input_streams;
  std::vector<TableData> input_tables;
  for (int input_stream_id = 0; input_stream_id < input_data_locations.size();
       input_stream_id++) {
    auto current_input_table = data_manager.ParseDataFromCSV(
        input_data_locations[input_stream_id].second);
    WriteInputData(input_data_locations[input_stream_id].first,
                   current_input_table);
    input_streams.push_back(
        {input_stream_id, GetRecordSize(current_input_table),
         static_cast<int>(current_input_table.table_data_vector.size() /
                          GetRecordSize(current_input_table)),
         input_data_locations[input_stream_id].first->GetPhysicalAddress()});
    input_tables.push_back(current_input_table);
  }

  std::vector<StreamInitialisationData> output_streams;
  std::vector<TableData> output_tables;
  for (int output_stream_id = 0;
       output_stream_id < output_data_locations.size(); output_stream_id++) {
    TableData current_resulting_table;
    current_resulting_table.table_column_label_vector =
        input_tables[output_stream_id].table_column_label_vector;
    output_streams.push_back(
        {output_stream_id, GetRecordSize(current_resulting_table), 0,
         output_data_locations[output_stream_id].first->GetPhysicalAddress()});
    output_tables.push_back(current_resulting_table);
  }

  fpga_manager.SetupQueryAcceleration(input_streams, output_streams);

  std::cout << "Running query!" << std::endl;
  auto result_sizes = fpga_manager.RunQueryAcceleration();
  std::cout << "Query done!" << std::endl;

  for (int output_stream_id = 0;
       output_stream_id < output_data_locations.size(); output_stream_id++) {
    ReadOutputData(output_data_locations[output_stream_id].first,
                   output_tables[output_stream_id],
                   result_sizes[output_stream_id]);
    std::cout << "Result has " << result_sizes[output_stream_id] << " rows!"
              << std::endl;

    auto expected_table = data_manager.ParseDataFromCSV(
        output_data_locations[output_stream_id].second);
    CheckTableData(expected_table, output_tables[output_stream_id]);
  }
}

auto main() -> int {
  std::cout << "Starting up!" << std::endl;
  DataManager data_manager("data_config.ini");
  MemoryManager memory_manager("DSPI_filtering");
  FPGAManager fpga_manager(&memory_manager);

  // With current settings 8 allocations is the max - after then have to start
  // reusing or sharing

  std::vector<std::pair<std::unique_ptr<MemoryBlockInterface>, std::string>>
      input_data_locations;
  input_data_locations.push_back(std::move(std::make_pair(
      memory_manager.AllocateMemoryBlock(), std::string("CAR_DATA.csv"))));
  input_data_locations.push_back(std::move(std::make_pair(
      memory_manager.AllocateMemoryBlock(), std::string("CAR_DATA.csv"))));
  input_data_locations.push_back(std::move(std::make_pair(
      memory_manager.AllocateMemoryBlock(), std::string("CUSTOMER_DATA.csv"))));

  std::vector<std::pair<std::unique_ptr<MemoryBlockInterface>, std::string>>
      output_data_locations;
  output_data_locations.push_back(
      std::move(std::make_pair(memory_manager.AllocateMemoryBlock(),
                               std::string("CAR_FILTER_DATA.csv"))));
  output_data_locations.push_back(std::move(std::make_pair(
      memory_manager.AllocateMemoryBlock(), std::string("CAR_DATA.csv"))));
  output_data_locations.push_back(std::move(std::make_pair(
      memory_manager.AllocateMemoryBlock(), std::string("CUSTOMER_DATA.csv"))));

  RunQueryWithData(data_manager, fpga_manager, input_data_locations,
                   output_data_locations);

  input_data_locations.clear();
  output_data_locations.clear();

  input_data_locations.push_back(std::move(std::make_pair(
      memory_manager.AllocateMemoryBlock(), std::string("CAR_DATA.csv"))));
  output_data_locations.push_back(
      std::move(std::make_pair(memory_manager.AllocateMemoryBlock(),
                               std::string("CAR_FILTER_DATA.csv"))));

  RunQueryWithData(data_manager, fpga_manager, input_data_locations,
                   output_data_locations);
  return 0;
}
