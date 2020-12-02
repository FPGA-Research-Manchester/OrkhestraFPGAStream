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
#include "stream_data_parameters.hpp"
#include "stream_initialisation_data.hpp"
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

// Debug method
void PrintInputDataOut(
    DataManager& data_manager,
    const std::vector<StreamInitialisationData>& input_data_locations) {
  for (const auto& input_data_location : input_data_locations) {
    auto current_input_table = data_manager.ParseDataFromCSV(
        input_data_location.stream_data_file_name);
    const auto& table_from_file = current_input_table.table_data_vector;
    volatile uint32_t* input =
        input_data_location.memory_block->GetVirtualAddress();
    auto input_table =
        std::vector<uint32_t>(input, input + (table_from_file.size() / 2));
    current_input_table.table_data_vector = input_table;
    DataManager::PrintTableData(current_input_table);
    input_table = std::vector<uint32_t>(input + (table_from_file.size() / 2),
                                        input + (table_from_file.size()));
    current_input_table.table_data_vector = input_table;
    DataManager::PrintTableData(current_input_table);
  }
}

auto GetRecordSize(const TableData& input_table) -> int {
  int record_size = 0;
  for (const auto& column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void CheckTableData(const TableData& expected_table,
                    const TableData& resulting_table) {
  if (expected_table == resulting_table) {
    std::cout << "Query results are correct!" << std::endl;
  } else {
    std::cout << "Incorrect query results:" << std::endl;
    std::cout << expected_table.table_data_vector.size() /
                     GetRecordSize(expected_table)
              << std::endl;
    // DataManager::PrintTableData(expected_table);
    std::cout << "vs:" << std::endl;
    std::cout << resulting_table.table_data_vector.size() /
                     GetRecordSize(resulting_table)
              << std::endl;
    DataManager::PrintTableData(resulting_table);
  }
}

void ReadOutputData(const std::unique_ptr<MemoryBlockInterface>& output_device,
                    TableData& resulting_table, const int& result_size) {
  volatile uint32_t* output = output_device->GetVirtualAddress();
  resulting_table.table_data_vector = std::vector<uint32_t>(
      output, output + (result_size * GetRecordSize(resulting_table)));
}

void WriteInputData(const std::unique_ptr<MemoryBlockInterface>& input_device,
                    const TableData& input_table) {
  volatile uint32_t* input = input_device->GetVirtualAddress();
  for (int i = 0; i < input_table.table_data_vector.size(); i++) {
    input[i] = input_table.table_data_vector[i];
  }
}

void ReadInputTables(
    const std::vector<StreamInitialisationData>& input_data_locations,
    DataManager& data_manager,
    std::vector<StreamDataParameters>& input_streams) {
  for (const auto& stream_initialisation_data : input_data_locations) {
    auto current_input_table = data_manager.ParseDataFromCSV(
        stream_initialisation_data.stream_data_file_name);
    WriteInputData(stream_initialisation_data.memory_block,
                   current_input_table);
    input_streams.push_back(
        {stream_initialisation_data.stream_id,
         GetRecordSize(current_input_table),
         static_cast<int>(current_input_table.table_data_vector.size() /
                          GetRecordSize(current_input_table)),
         stream_initialisation_data.memory_block->GetPhysicalAddress()});
  }
}

void ReadExpectedTables(
    const std::vector<StreamInitialisationData>& output_data_locations,
    DataManager& data_manager, std::vector<TableData>& expected_output_tables,
    std::vector<StreamDataParameters>& output_streams,
    std::vector<TableData>& output_tables) {
  for (const auto& stream_initialisation_data : output_data_locations) {
    auto expected_table = data_manager.ParseDataFromCSV(
        stream_initialisation_data.stream_data_file_name);
    expected_output_tables.push_back(expected_table);
    TableData current_resulting_table;
    current_resulting_table.table_column_label_vector =
        expected_table.table_column_label_vector;
    output_streams.push_back(
        {stream_initialisation_data.stream_id,
         GetRecordSize(current_resulting_table), 0,
         stream_initialisation_data.memory_block->GetPhysicalAddress()});
    output_tables.push_back(current_resulting_table);
  }
}

void RunQueryWithData(
    DataManager& data_manager, FPGAManager& fpga_manager,
    const std::vector<StreamInitialisationData>& input_data_locations,
    const std::vector<StreamInitialisationData>& output_data_locations,
    bool is_filtering) {
  std::vector<StreamDataParameters> input_streams;
  ReadInputTables(input_data_locations, data_manager, input_streams);

  std::vector<StreamDataParameters> output_streams;
  std::vector<TableData> output_tables;
  std::vector<TableData> expected_output_tables;
  ReadExpectedTables(output_data_locations, data_manager,
                     expected_output_tables, output_streams, output_tables);

  fpga_manager.SetupQueryAcceleration(input_streams, output_streams,
                                      is_filtering);

  std::cout << "Running query!" << std::endl;
  auto result_sizes = fpga_manager.RunQueryAcceleration();
  std::cout << "Query done!" << std::endl;

  for (int i = 0; i < output_data_locations.size(); i++) {
    ReadOutputData(output_data_locations[i].memory_block, output_tables[i],
                   result_sizes[output_data_locations[i].stream_id]);
    std::cout << "Result has "
              << result_sizes[output_data_locations[i].stream_id] << " rows!"
              << std::endl;

    CheckTableData(expected_output_tables[i], output_tables[i]);
  }
}

void FillDataLocationsVector(
    std::vector<StreamInitialisationData>& data_locations,
    MemoryManager* memory_manager,
    const std::vector<std::string>& file_name_vector,
    const std::vector<int>& stream_id_vector) {
  for (int i = 0; i < file_name_vector.size(); i++) {
    data_locations.push_back({stream_id_vector[i], file_name_vector[i],
                              memory_manager->AllocateMemoryBlock()});
  }
}

// With current settings 8 allocations is the max - after then have to start
// reusing or sharing
auto main() -> int {
  const int module_size = 1024 * 1024;

  std::cout << "Starting up!" << std::endl;
  DataManager data_manager("data_config.ini");

  bool is_filtering = false;

  std::vector<StreamInitialisationData> input_data_locations;
  std::vector<StreamInitialisationData> output_data_locations;

  if (is_filtering) {
    MemoryManager memory_manager("DSPI_filtering", 2 * module_size);
    FPGAManager fpga_manager(&memory_manager);

    FillDataLocationsVector(
        input_data_locations, &memory_manager,
        {"CAR_DATA.csv", "CAR_DATA.csv", "CUSTOMER_DATA.csv"}, {0, 1, 2});
    FillDataLocationsVector(
        output_data_locations, &memory_manager,
        {"CAR_FILTER_DATA.csv", "CAR_DATA.csv", "CUSTOMER_DATA.csv"},
        {0, 1, 2});

    RunQueryWithData(data_manager, fpga_manager, input_data_locations,
                     output_data_locations, is_filtering);

    input_data_locations.clear();
    output_data_locations.clear();

    FillDataLocationsVector(input_data_locations, &memory_manager,
                            {"CAR_DATA.csv"}, {0});
    FillDataLocationsVector(output_data_locations, &memory_manager,
                            {"CAR_FILTER_DATA.csv"}, {0});

    RunQueryWithData(data_manager, fpga_manager, input_data_locations,
                     output_data_locations, is_filtering);
  } else {
    MemoryManager memory_manager("DSPI_joining", 2 * module_size);
    FPGAManager fpga_manager(&memory_manager);

    FillDataLocationsVector(
        input_data_locations, &memory_manager,
        {"CAR_DATA.csv", "CUSTOMER_DATA_FOR_JOIN.csv", "CAR_FILTER_DATA.csv"},
        {0, 1, 2});
    FillDataLocationsVector(output_data_locations, &memory_manager,
                            {"JOIN_DATA.csv", "CAR_FILTER_DATA.csv"}, {0, 2});

    RunQueryWithData(data_manager, fpga_manager, input_data_locations,
                     output_data_locations, is_filtering);
  }
  return 0;
}
