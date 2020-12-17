#include "query_manager.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>

#include "memory_block_interface.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

// Debug method
void QueryManager::PrintInputDataOut(
    DataManager& data_manager,
    const std::vector<StreamInitialisationData>& input_data_locations) {
  for (const auto& input_data_location : input_data_locations) {
    std::cout << "======================="
              << input_data_location.stream_data_file_name
              << "=======================" << std::endl;
    auto current_input_table = data_manager.ParseDataFromCSV(
        input_data_location.stream_data_file_name);
    const auto& table_from_file = current_input_table.table_data_vector;
    volatile uint32_t* input =
        input_data_location.memory_block->GetVirtualAddress();
    auto input_table =
        std::vector<uint32_t>(input, input + (table_from_file.size()));
    current_input_table.table_data_vector = input_table;
    DataManager::PrintTableData(current_input_table);
    std::cout << std::endl;
  }
}

auto QueryManager::GetRecordSizeFromTable(const TableData& input_table) -> int {
  int record_size = 0;
  for (const auto& column_type : input_table.table_column_label_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void QueryManager::CheckTableData(const TableData& expected_table,
                                  const TableData& resulting_table) {
  if (expected_table == resulting_table) {
    std::cout << "Query results are correct!" << std::endl;
  } else {
    std::cout << "Incorrect query results:" << std::endl;
    std::cout << expected_table.table_data_vector.size() /
                     GetRecordSizeFromTable(expected_table)
              << std::endl;
    // DataManager::PrintTableData(expected_table);
    std::cout << "vs:" << std::endl;
    std::cout << resulting_table.table_data_vector.size() /
                     GetRecordSizeFromTable(resulting_table)
              << std::endl;
    DataManager::PrintTableData(resulting_table);
  }
}

void QueryManager::ReadOutputDataFromMemoryBlock(
    const std::unique_ptr<MemoryBlockInterface>& output_device,
    TableData& resulting_table, const int& result_size) {
  volatile uint32_t* output = output_device->GetVirtualAddress();
  resulting_table.table_data_vector = std::vector<uint32_t>(
      output, output + (result_size * GetRecordSizeFromTable(resulting_table)));
}

void QueryManager::WriteInputDataToMemoryBlock(
    const std::unique_ptr<MemoryBlockInterface>& input_device,
    const TableData& input_table) {
  volatile uint32_t* input = input_device->GetVirtualAddress();
  for (int i = 0; i < input_table.table_data_vector.size(); i++) {
    input[i] = input_table.table_data_vector[i];
  }
}

void QueryManager::ReadInputTables(
    const std::vector<StreamInitialisationData>& input_data_locations,
    DataManager& data_manager,
    std::vector<StreamDataParameters>& input_streams) {
  for (const auto& stream_initialisation_data : input_data_locations) {
    auto current_input_table = data_manager.ParseDataFromCSV(
        stream_initialisation_data.stream_data_file_name);
    WriteInputDataToMemoryBlock(stream_initialisation_data.memory_block,
                                current_input_table);
    input_streams.push_back(
        {stream_initialisation_data.stream_id,
         GetRecordSizeFromTable(current_input_table),
         static_cast<int>(current_input_table.table_data_vector.size() /
                          GetRecordSizeFromTable(current_input_table)),
         stream_initialisation_data.memory_block->GetPhysicalAddress()});
  }
}

void QueryManager::ReadExpectedTables(
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
         GetRecordSizeFromTable(current_resulting_table), 0,
         stream_initialisation_data.memory_block->GetPhysicalAddress()});
    output_tables.push_back(current_resulting_table);
  }
}

void QueryManager::RunQueryWithData(
    DataManager& data_manager, FPGAManager& fpga_manager,
    const std::vector<StreamInitialisationData>& input_data_locations,
    const std::vector<StreamInitialisationData>& output_data_locations,
    operation_types::QueryOperation operation) {
  std::vector<StreamDataParameters> input_streams;
  ReadInputTables(input_data_locations, data_manager, input_streams);

  std::vector<StreamDataParameters> output_streams;
  std::vector<TableData> output_tables;
  std::vector<TableData> expected_output_tables;
  ReadExpectedTables(output_data_locations, data_manager,
                     expected_output_tables, output_streams, output_tables);

  fpga_manager.SetupQueryAcceleration(input_streams, output_streams, operation);

  std::cout << "Running query!" << std::endl;
  auto result_sizes = fpga_manager.RunQueryAcceleration();
  std::cout << "Query done!" << std::endl;

  for (int i = 0; i < output_data_locations.size(); i++) {
    ReadOutputDataFromMemoryBlock(
        output_data_locations[i].memory_block, output_tables[i],
        result_sizes[output_data_locations[i].stream_id]);
    std::cout << "Result has "
              << result_sizes[output_data_locations[i].stream_id] << " rows!"
              << std::endl;

    CheckTableData(expected_output_tables[i], output_tables[i]);
  }
}

void QueryManager::FillDataLocationsVector(
    std::vector<StreamInitialisationData>& data_locations,
    MemoryManager* memory_manager,
    const std::vector<std::string>& file_name_vector,
    const std::vector<int>& stream_id_vector) {
  for (int i = 0; i < file_name_vector.size(); i++) {
    data_locations.push_back({stream_id_vector[i], file_name_vector[i],
                              memory_manager->GetAvailableMemoryBlock()});
  }
}

void QueryManager::RunQueries(std::vector<QueryNode> starting_query_nodes) {
  std::cout << "Starting up!" << std::endl;
  DataManager data_manager("data_config.ini");

  // Should be put somewhere else!
  std::map<operation_types::QueryOperation, std::string>
      corresponding_accelerator_bitstreams = {
          {operation_types::QueryOperation::Filter, "DSPI_filtering"},
          {operation_types::QueryOperation::Join, "DSPI_joining"},
          {operation_types::QueryOperation::MergeSort, "DSPI_merge_sorting"},
          {operation_types::QueryOperation::PassThrough, "need_to_fix"}}; // Fix pass through operations

  // Make it possible to run multiple nodes at once - Like the passthrough ones
  for (const auto& starting_query_node : starting_query_nodes) {
    // Shouldn't be hard coded!
    const int module_count = 2;
    auto bitstreams_iterator = corresponding_accelerator_bitstreams.find(
        starting_query_node.operation_type);
    std::string accelerator_to_load;
    if (bitstreams_iterator != corresponding_accelerator_bitstreams.end()) {
      accelerator_to_load = bitstreams_iterator->second;
    } else {
      throw std::runtime_error("Unsopported operation!");
    }
    MemoryManager memory_manager(
        accelerator_to_load,
        module_count * query_acceleration_constants::kModuleSize);
    FPGAManager fpga_manager(&memory_manager);

    // These ID allocations need to be improved
    std::vector<int> input_stream_id_vector;
    for (int stream_id = 0;
         stream_id < starting_query_node.input_data_definition_files.size();
         stream_id++) {
      input_stream_id_vector.push_back(stream_id);
    }

    std::vector<StreamInitialisationData> input_data_locations;
    FillDataLocationsVector(input_data_locations, &memory_manager,
                            starting_query_node.input_data_definition_files,
                            input_stream_id_vector);

    std::vector<int> output_stream_id_vector;
    for (int stream_id = 0;
         stream_id < starting_query_node.output_data_definition_files.size();
         stream_id++) {
      output_stream_id_vector.push_back(stream_id);
    }

    std::vector<StreamInitialisationData> output_data_locations;
    FillDataLocationsVector(output_data_locations, &memory_manager,
                            starting_query_node.output_data_definition_files,
                            output_stream_id_vector);

    RunQueryWithData(data_manager, fpga_manager, input_data_locations,
                     output_data_locations, starting_query_node.operation_type);

    // Free available memory blocks
  }

  // Hardcoded queries here which should be supported by the flow above ASAP!
  MemoryManager memory_manager("DSPI_double_merge_sorting",
                               3 * query_acceleration_constants::kModuleSize);
  FPGAManager fpga_manager(&memory_manager);

  std::vector<StreamInitialisationData> input_data_locations;
  FillDataLocationsVector(input_data_locations, &memory_manager,
                          {"CAR_DATA_HALF_SORTED_8K_63WAY.csv"}, {0});
  std::vector<StreamInitialisationData> output_data_locations;
  FillDataLocationsVector(output_data_locations, &memory_manager,
                          {"CAR_DATA_SORTED_8K.csv"}, {0});

  RunQueryWithData(data_manager, fpga_manager, input_data_locations,
                   output_data_locations,
                   operation_types::QueryOperation::MergeSort);
}
