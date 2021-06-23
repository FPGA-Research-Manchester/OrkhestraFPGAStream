#include "query_manager.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>

#include "accelerated_query_node.hpp"
#include "data_manager.hpp"
#include "elastic_module_checker.hpp"
#include "fpga_manager.hpp"
#include "id_manager.hpp"
#include "logger.hpp"
#include "memory_block_interface.hpp"
#include "node_scheduler.hpp"
#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"
#include "query_scheduling_data.hpp"
#include "stream_parameter_calculator.hpp"
#include "table_manager.hpp"
#include "util.hpp"

using namespace dbmstodspi::query_managing;
using dbmstodspi::data_managing::table_data::TableData;
using dbmstodspi::fpga_managing::query_acceleration_constants::
    kMaxIOStreamCount;
using dbmstodspi::logger::Log;
using dbmstodspi::logger::LogLevel;
using dbmstodspi::logger::ShouldLog;
using dbmstodspi::query_managing::query_scheduling_data::kIOStreamParamDefs;
using dbmstodspi::util::CreateReferenceVector;

void QueryManager::CheckTableData(const TableData& expected_table,
                                  const TableData& resulting_table) {
  if (expected_table == resulting_table) {
    Log(LogLevel::kDebug, "Query results are correct!");
  } else {
    Log(LogLevel::kError,
        "Incorrect query results: " +
            std::to_string(
                expected_table.table_data_vector.size() /
                TableManager::GetRecordSizeFromTable(expected_table)) +
            " vs " +
            std::to_string(
                resulting_table.table_data_vector.size() /
                TableManager::GetRecordSizeFromTable(resulting_table)) +
            " rows!");
    data_managing::DataManager::PrintTableData(resulting_table);
  }
}

void dbmstodspi::query_managing::QueryManager::InitialiseMemoryBlockVector(
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        memory_blocks,
    int stream_count, std::string node_name) {
  std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>
      empty_vector(stream_count);
  std::fill(empty_vector.begin(), empty_vector.end(), nullptr);
  memory_blocks.insert({node_name, empty_vector});
}

auto dbmstodspi::query_managing::QueryManager::GetRecordSizeFromParameters(
    const DataManager& data_manager,
    std::vector<std::vector<int>> node_parameters, int stream_index) -> int {
  std::vector<ColumnDataType> column_data_types;
  for (const auto& type_int_value :
       node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                          kIOStreamParamDefs.kDataTypesOffset)) {
    column_data_types.push_back(static_cast<ColumnDataType>(type_int_value));
  }
  auto column_defs_vector = data_manager.GetHeaderColumnVector(
      column_data_types,
      node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                         kIOStreamParamDefs.kDataSizesOffset));

  int record_size = 0;
  for (const auto& column_type : column_defs_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

// Create map with correct amount of elements locations and data reuse links
void dbmstodspi::query_managing::QueryManager::FindOutputNodes(
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
        scheduled_nodes,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        input_memory_blocks,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        output_memory_blocks,
    std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links) {
  for (const auto& node : scheduled_nodes) {
    // Input could be defined from previous runs
    if (input_memory_blocks.find(node->node_name) ==
        input_memory_blocks.end()) {
      InitialiseMemoryBlockVector(input_memory_blocks,
                                  node->previous_nodes.size(), node->node_name);
    }

    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>
        empty_vector(node->next_nodes.size());
    std::fill(empty_vector.begin(), empty_vector.end(), nullptr);
    output_memory_blocks.insert({node->node_name, empty_vector});

    for (const auto& output_node : node->next_nodes) {
      if (input_memory_blocks.find(output_node->node_name) ==
          input_memory_blocks.end()) {
        InitialiseMemoryBlockVector(input_memory_blocks,
                                    output_node->previous_nodes.size(),
                                    output_node->node_name);
      }
    }

    auto link_search = reuse_links.find(node->node_name);
    if (link_search == reuse_links.end()) {
      // Do not populate memory reuse map for now
    }
  }
}

void QueryManager::AllocateOutputMemoryBlocks(
    fpga_managing::MemoryManager memory_manager,
    const DataManager& data_manager,
    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        output_memory_blocks,
    const query_scheduling_data::QueryNode& node, std::vector<int>& row_count,
    std::vector<int>& record_sizes) {
  int stream_count = node.next_nodes.size();
  record_sizes.reserve(stream_count);
  row_count.reserve(stream_count);
  std::fill(row_count.begin(), row_count.end(), 0);
  for (int stream_index = 0; stream_index < stream_count; stream_index++) {
    if (!node.next_nodes[stream_index]) {
      output_memory_blocks[stream_index] =
          (std::move(memory_manager.GetAvailableMemoryBlock()));
    }
    record_sizes.push_back(
        GetRecordSizeFromParameters(data_manager, node.operation_parameters.output_stream_parameters, stream_index));
  }
}
void QueryManager::AllocateInputMemoryBlocks(
    fpga_managing::MemoryManager memory_manager,
    const DataManager& data_manager,
    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        input_memory_blocks,
    const query_scheduling_data::QueryNode& node, std::vector<int>& row_count,
    std::vector<int>& record_sizes) {
  for (int stream_index = 0; stream_index < node.previous_nodes.size();
       stream_index++) {
    auto observed_node = node.previous_nodes[stream_index].lock();
    if (!observed_node && !input_memory_blocks[stream_index]) {
      input_memory_blocks[stream_index] =
          (std::move(memory_manager.GetAvailableMemoryBlock()));
      // WRITE and get sizes!
      assert(false);
    }
  }
}

auto dbmstodspi::query_managing::QueryManager::CreateStreamParams(
    const DataManager& data_manager, const std::vector<int>& stream_ids,
    const std::vector<std::vector<int>>& node_parameters,
    const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        allocated_memory_blocks,
    const std::vector<int>& row_counts, const std::vector<int>& record_sizes)
    -> std::vector<fpga_managing::StreamDataParameters> {
  std::vector<fpga_managing::StreamDataParameters> parameters_for_acceleration;

  for (int stream_index = 0; stream_index < stream_ids.size(); stream_index++) {
    volatile uint32_t* physical_address_ptr = nullptr;
    if (allocated_memory_blocks[stream_index]) {
      physical_address_ptr =
          allocated_memory_blocks[stream_index]->GetPhysicalAddress();
    }

    fpga_managing::StreamDataParameters current_stream_parameters = {
        stream_ids[stream_index], record_sizes[stream_index],
        row_counts[stream_index], physical_address_ptr,
        node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                           kIOStreamParamDefs.kProjectionOffset)};

    parameters_for_acceleration.push_back(current_stream_parameters);
  }

  return parameters_for_acceleration;
}

void dbmstodspi::query_managing::QueryManager::StoreStreamResultPrameters(
    std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::vector<int> stream_ids,
    const query_scheduling_data::QueryNode& node,
    const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        allocated_memory_blocks) {}

void dbmstodspi::query_managing::QueryManager::ProcessResults(
    std::array<int, kMaxIOStreamCount> result_sizes,
    std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters) {}

void dbmstodspi::query_managing::QueryManager::FreeMemoryBlocks(
    fpga_managing::MemoryManager memory_manager,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        input_memory_blocks,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        output_memory_blocks,
    std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links) {}

void QueryManager::RunQueries(
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
        starting_query_nodes,
    const Config& config) {
  Log(LogLevel::kTrace, "Starting up!");
  data_managing::DataManager data_manager(config.data_sizes);
  fpga_managing::MemoryManager memory_manager;
  fpga_managing::FPGAManager fpga_manager(&memory_manager);

  auto query_node_runs_queue = NodeScheduler::FindAcceleratedQueryNodeSets(
      std::move(starting_query_nodes), config.accelerator_library,
      config.module_library);
  Log(LogLevel::kTrace, "Scheduling done!");

  std::map<std::string,
           std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>
      input_memory_blocks;
  std::map<std::string,
           std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>
      output_memory_blocks;
  std::map<std::string, std::map<int, MemoryReuseTargets>> reuse_links;

  while (!query_node_runs_queue.empty()) {
    std::map<std::string, std::vector<StreamResultParameters>>
        result_parameters;
    std::vector<fpga_managing::AcceleratedQueryNode> query_nodes;
    std::map<std::string, std::vector<int>> output_ids;
    std::map<std::string, std::vector<int>> input_ids;

    const auto executable_query_nodes = query_node_runs_queue.front().second;
    const auto bitstream_file_name =
        config.accelerator_library.at(query_node_runs_queue.front().first);
    query_node_runs_queue.pop();

    memory_manager.LoadBitstreamIfNew(
        bitstream_file_name,
        config.required_memory_space.at(bitstream_file_name));

    FindOutputNodes(executable_query_nodes, input_memory_blocks,
                    output_memory_blocks, reuse_links);

    IDManager::AllocateStreamIDs(CreateReferenceVector(executable_query_nodes),
                                 input_ids, output_ids);

    for (const auto& node : executable_query_nodes) {
      std::vector<int> input_row_counts;
      std::vector<int> output_row_counts;
      std::vector<int> input_record_sizes;
      std::vector<int> output_record_sizes;
      AllocateInputMemoryBlocks(memory_manager, data_manager,
                                input_memory_blocks[node->node_name], *node,
                                input_row_counts, input_record_sizes);
      AllocateOutputMemoryBlocks(memory_manager, data_manager,
                                 output_memory_blocks[node->node_name], *node,
                                 output_row_counts, output_record_sizes);

      auto input_params =
          CreateStreamParams(data_manager, input_ids[node->node_name],
                             node->operation_parameters.input_stream_parameters,
                             input_memory_blocks[node->node_name],
                             input_row_counts, input_record_sizes);
      auto output_params = CreateStreamParams(
          data_manager, output_ids[node->node_name],
          node->operation_parameters.output_stream_parameters,
          output_memory_blocks[node->node_name], output_row_counts,
          output_record_sizes);

      ElasticModuleChecker::CheckElasticityNeeds(
          input_params, node->operation_type,
          node->operation_parameters.operation_parameters);
      query_nodes.push_back({std::move(input_params), std::move(output_params),
                             node->operation_type, node->module_location,
                             node->operation_parameters.operation_parameters});
      StoreStreamResultPrameters(result_parameters, output_ids[node->node_name],
                                 *node, output_memory_blocks[node->node_name]);
    }

    Log(LogLevel::kTrace, "Setup query!");
    fpga_manager.SetupQueryAcceleration(query_nodes);
    Log(LogLevel::kTrace, "Running query!");
    auto result_sizes = fpga_manager.RunQueryAcceleration();
    Log(LogLevel::kTrace, "Query done!");

    // TODO needs data_manager for reading
    ProcessResults(result_sizes, result_parameters);
    FreeMemoryBlocks(memory_manager, input_memory_blocks, output_memory_blocks,
                     reuse_links);
  }
}

// void QueryManager::RunQueries1(
//    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
//        starting_query_nodes,
//    const Config& config) {
//  Log(LogLevel::kTrace, "Starting up!");
//  data_managing::DataManager data_manager(config.data_sizes);
//  fpga_managing::MemoryManager memory_manager;
//  fpga_managing::FPGAManager fpga_manager(&memory_manager);
//
//  auto query_node_runs_queue = NodeScheduler::FindAcceleratedQueryNodeSets(
//      std::move(starting_query_nodes), config.accelerator_library,
//      config.module_library);
//  Log(LogLevel::kTrace, "Scheduling done!");
//
//  while (!query_node_runs_queue.empty()) {
//    const auto executable_query_nodes = query_node_runs_queue.front().second;
//
//    const auto& bitstream_file_name =
//        config.accelerator_library.at(query_node_runs_queue.front().first);
//    query_node_runs_queue.pop();
//
//    memory_manager.LoadBitstreamIfNew(
//        bitstream_file_name,
//        config.required_memory_space.at(bitstream_file_name));
//
//    IDManager id_manager;
//    std::vector<std::vector<int>> output_ids;
//    std::vector<std::vector<int>> input_ids;
//    std::vector<
//        std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>
//        input_memory_blocks;
//    std::vector<
//        std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>
//        output_memory_blocks;
//    std::vector<TableData> expected_output_tables(
//        fpga_managing::query_acceleration_constants::kMaxIOStreamCount);
//    std::vector<std::string> expected_output_files(
//        fpga_managing::query_acceleration_constants::kMaxIOStreamCount);
//    std::vector<fpga_managing::AcceleratedQueryNode> query_nodes;
//
//    id_manager.AllocateStreamIDs(CreateReferenceVector(executable_query_nodes),
//                                 input_ids, output_ids);
//
//    for (int node_index = 0; node_index < executable_query_nodes.size();
//         node_index++) {
//      auto current_node = *executable_query_nodes.at(node_index);
//
//      // Allocate memory blocks
//      std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>
//          allocated_input_memory_blocks;
//      for (const auto& linked_node : current_node.previous_nodes) {
//        auto observed_node = linked_node.lock();
//        if (!observed_node) {
//          allocated_input_memory_blocks.push_back(
//              memory_manager.GetAvailableMemoryBlock());
//        } else {
//          allocated_input_memory_blocks.push_back(nullptr);
//        }
//      }
//
//      std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>
//          allocated_output_memory_blocks;
//      for (const auto& linked_node : current_node.next_nodes) {
//        if (!linked_node) {
//          allocated_output_memory_blocks.push_back(
//              memory_manager.GetAvailableMemoryBlock());
//        } else {
//          allocated_output_memory_blocks.push_back(nullptr);
//        }
//      }
//
//      // Get parameters and write input to allocated blocks
//      std::vector<fpga_managing::StreamDataParameters>
//      input_stream_parameters; TableManager::ReadInputTables(
//          input_stream_parameters, data_manager,
//          current_node.input_data_definition_files, input_ids[node_index],
//          allocated_input_memory_blocks,
//          current_node.operation_parameters.input_stream_parameters);
//
//      std::vector<fpga_managing::StreamDataParameters>
//      output_stream_parameters; TableManager::ReadExpectedTables(
//          output_stream_parameters, data_manager,
//          current_node.output_data_definition_files, output_ids[node_index],
//          allocated_output_memory_blocks, expected_output_tables,
//          expected_output_files,
//          current_node.operation_parameters.output_stream_parameters);
//
//      // Check if the loaded modules are correct based on the input.
//      ElasticModuleChecker::CheckElasticityNeeds(
//          input_stream_parameters, current_node.operation_type,
//          current_node.operation_parameters.operation_parameters);
//
//      query_nodes.push_back(
//          {std::move(input_stream_parameters),
//           std::move(output_stream_parameters), current_node.operation_type,
//           current_node.module_location,
//           current_node.operation_parameters.operation_parameters});
//
//      // Keep memory blocks during the query execution
//      input_memory_blocks.push_back(std::move(allocated_input_memory_blocks));
//      output_memory_blocks.push_back(std::move(allocated_output_memory_blocks));
//    }
//
//    // Run query
//    Log(LogLevel::kTrace, "Setup query!");
//    fpga_manager.SetupQueryAcceleration(query_nodes);
//    Log(LogLevel::kTrace, "Running query!");
//    auto result_sizes = fpga_manager.RunQueryAcceleration();
//    Log(LogLevel::kTrace, "Query done!");
//
//    // Check results & free memory
//    std::vector<TableData> output_tables = expected_output_tables;
//    for (int node_index = 0; node_index < query_nodes.size(); node_index++) {
//      TableManager::ReadResultTables(query_nodes[node_index].output_streams,
//                                     output_tables, result_sizes,
//                                     output_memory_blocks[node_index]);
//      for (int stream_index = 0; stream_index < output_ids[node_index].size();
//           stream_index++) {
//        if (output_memory_blocks[node_index][stream_index]) {
//          Log(LogLevel::kDebug,
//              "Result has " +
//                  std::to_string(
//                      result_sizes[output_ids[node_index][stream_index]]) +
//                  " rows!");
//
//          if (expected_output_tables[output_ids[node_index][stream_index]]
//                  .table_data_vector.empty() &&
//              result_sizes[output_ids[node_index][stream_index]] != 0) {
//            TableManager::WriteResultTableFile(
//                output_tables[output_ids[node_index][stream_index]],
//                expected_output_files[output_ids[node_index][stream_index]]);
//          } else {
//            CheckTableData(
//                expected_output_tables[output_ids[node_index][stream_index]],
//                output_tables[output_ids[node_index][stream_index]]);
//          }
//        }
//      }
//
//      // Free all memory for now.
//      for (auto& memory_pointer : input_memory_blocks[node_index]) {
//        if (memory_pointer) {
//          memory_manager.FreeMemoryBlock(std::move(memory_pointer));
//        }
//      }
//      for (auto& memory_pointer : output_memory_blocks[node_index]) {
//        if (memory_pointer) {
//          memory_manager.FreeMemoryBlock(std::move(memory_pointer));
//        }
//      }
//    }
//  }
//}