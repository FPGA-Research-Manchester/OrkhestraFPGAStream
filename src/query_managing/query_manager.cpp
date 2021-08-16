#include "query_manager.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stdexcept>

#include <chrono>

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
using dbmstodspi::util::FindPositionInVector;

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

void QueryManager::InitialiseMemoryBlockVector(
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        memory_blocks,
    int stream_count, std::string node_name) {
  std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>
      empty_vector(stream_count);
  std::fill(empty_vector.begin(), empty_vector.end(), nullptr);
  memory_blocks.insert({node_name, std::move(empty_vector)});
}

void QueryManager::InitialiseStreamSizeVector(
    std::map<std::string, std::vector<RecordSizeAndCount>>& stream_sizes,
    int stream_count, std::string node_name) {
  std::vector<RecordSizeAndCount> empty_vector(stream_count);
  std::fill(empty_vector.begin(), empty_vector.end(), std::make_pair(0, 0));
  stream_sizes.insert({node_name, std::move(empty_vector)});
}

auto QueryManager::GetRecordSizeFromParameters(
    const DataManager& data_manager,
    const std::vector<std::vector<int>>& node_parameters, int stream_index)
    -> int {
  auto column_defs_vector = TableManager::GetColumnDefsVector(
      data_manager, node_parameters, stream_index);

  int record_size = 0;
  for (const auto& column_type : column_defs_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

// Create map with correct amount of elements locations and data reuse links
void QueryManager::InitialiseVectorSizes(
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        scheduled_nodes,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        input_memory_blocks,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>& input_stream_sizes,
    std::map<std::string, std::vector<RecordSizeAndCount>>&
        output_stream_sizes) {
  for (const auto& node : scheduled_nodes) {
    // Input could be defined from previous runs
    if (input_memory_blocks.find(node->node_name) ==
        input_memory_blocks.end()) {
      InitialiseMemoryBlockVector(input_memory_blocks,
                                  node->previous_nodes.size(), node->node_name);
      InitialiseStreamSizeVector(input_stream_sizes,
                                 node->previous_nodes.size(), node->node_name);
    }

    InitialiseMemoryBlockVector(output_memory_blocks, node->next_nodes.size(),
                                node->node_name);
    InitialiseStreamSizeVector(output_stream_sizes, node->previous_nodes.size(),
                               node->node_name);

    for (auto& output_node : node->next_nodes) {
      if (output_node) {
        if (input_memory_blocks.find(output_node->node_name) ==
            input_memory_blocks.end()) {
          InitialiseMemoryBlockVector(input_memory_blocks,
                                      output_node->previous_nodes.size(),
                                      output_node->node_name);
          InitialiseStreamSizeVector(input_stream_sizes,
                                     output_node->previous_nodes.size(),
                                     output_node->node_name);
        }
        if (std::find(scheduled_nodes.begin(), scheduled_nodes.end(),
                      output_node) == scheduled_nodes.end()) {
          output_node = nullptr;
        }
      }
    }
  }
}

void QueryManager::AllocateOutputMemoryBlocks(
    fpga_managing::MemoryManager& memory_manager,
    const DataManager& data_manager,
    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        output_memory_blocks,
    const query_scheduling_data::QueryNode& node,
    std::vector<RecordSizeAndCount>& output_stream_sizes) {
  for (int stream_index = 0; stream_index < node.next_nodes.size();
       stream_index++) {
    if (!node.next_nodes[stream_index]) {
      output_memory_blocks[stream_index] =
          std::move(memory_manager.GetAvailableMemoryBlock());
    }
    output_stream_sizes[stream_index].first = GetRecordSizeFromParameters(
        data_manager, node.operation_parameters.output_stream_parameters,
        stream_index);
  }
}
void QueryManager::AllocateInputMemoryBlocks(
    fpga_managing::MemoryManager& memory_manager,
    const DataManager& data_manager,
    std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        input_memory_blocks,
    const query_scheduling_data::QueryNode& node,
    const std::map<std::string, std::vector<RecordSizeAndCount>>&
        output_stream_sizes,
    std::vector<RecordSizeAndCount>& input_stream_sizes) {
  for (int stream_index = 0; stream_index < node.previous_nodes.size();
       stream_index++) {
    auto observed_node = node.previous_nodes[stream_index].lock();
    if (!observed_node && !input_memory_blocks[stream_index]) {
      input_memory_blocks[stream_index] =
          std::move(memory_manager.GetAvailableMemoryBlock());
      std::chrono::steady_clock::time_point begin =
          std::chrono::steady_clock::now();

      input_stream_sizes[stream_index] = TableManager::WriteDataToMemory1(
          data_manager, node.operation_parameters.input_stream_parameters,
          stream_index, input_memory_blocks[stream_index],
          node.input_data_definition_files[stream_index]);

      std::chrono::steady_clock::time_point end =
          std::chrono::steady_clock::now();
      Log(LogLevel::kInfo,
          "Read data time = " +
              std::to_string(
                  std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                        begin)
                      .count()) +
              "[ms]");
    } else if (observed_node) {  // Can also be moved to output memory blocks
                                 // allocation
      for (int current_node_index = 0;
           current_node_index < observed_node->next_nodes.size();
           current_node_index++) {
        if (node == *observed_node->next_nodes[current_node_index]) {
          input_stream_sizes[stream_index] = output_stream_sizes.at(
              observed_node->node_name)[current_node_index];
          break;
        }
      }
    }
  }
}

auto QueryManager::CreateStreamParams(
    const std::vector<int>& stream_ids,
    const std::vector<std::vector<int>>& node_parameters,
    const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        allocated_memory_blocks,
    const std::vector<RecordSizeAndCount>& stream_sizes)
    -> std::vector<fpga_managing::StreamDataParameters> {
  std::vector<fpga_managing::StreamDataParameters> parameters_for_acceleration;

  for (int stream_index = 0; stream_index < stream_ids.size(); stream_index++) {
    volatile uint32_t* physical_address_ptr = nullptr;
    if (allocated_memory_blocks[stream_index]) {
      physical_address_ptr =
          allocated_memory_blocks[stream_index]->GetPhysicalAddress();
    }

    int chunk_count = -1;
    auto chunk_count_def =
        node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                           kIOStreamParamDefs.kChunkCountOffset);
    if (!chunk_count_def.empty()) {
      chunk_count = chunk_count_def.at(0);
    }

    fpga_managing::StreamDataParameters current_stream_parameters = {
        stream_ids[stream_index],
        stream_sizes[stream_index].first,
        stream_sizes[stream_index].second,
        physical_address_ptr,
        node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                           kIOStreamParamDefs.kProjectionOffset),
        chunk_count};

    parameters_for_acceleration.push_back(current_stream_parameters);
  }

  return parameters_for_acceleration;
}

void QueryManager::StoreStreamResultPrameters(
    std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::vector<int>& stream_ids,
    const query_scheduling_data::QueryNode& node,
    const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
        allocated_memory_blocks) {
  std::vector<StreamResultParameters> result_parameters_vector;
  for (int stream_index = 0; stream_index < stream_ids.size(); stream_index++) {
    if (allocated_memory_blocks[stream_index]) {
      result_parameters_vector.emplace_back(
          stream_index, stream_ids[stream_index],
          node.output_data_definition_files[stream_index],
          node.is_checked[stream_index],
          node.operation_parameters.output_stream_parameters);
    }
  }
  result_parameters.insert({node.node_name, result_parameters_vector});
}

void QueryManager::ProcessResults(
    const DataManager& data_manager,
    const std::array<int, dbmstodspi::fpga_managing::
                              query_acceleration_constants::kMaxIOStreamCount>
        result_sizes,
    const std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::map<
        std::string,
        std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        allocated_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>&
        output_stream_sizes) {
  for (auto const& [node_name, result_parameter_vector] : result_parameters) {
    for (auto const& result_params : result_parameter_vector) {
      int record_count = result_sizes.at(result_params.output_id);
      Log(LogLevel::kDebug,
          node_name + "_" + std::to_string(result_params.stream_index) +
              " has " + std::to_string(record_count) + " rows!");
      output_stream_sizes[node_name][result_params.stream_index].second =
          record_count;
      if (!result_params.filename.empty()) {
        if (result_params.check_results) {
          CheckResults(
              data_manager,
              allocated_memory_blocks.at(node_name)[result_params.stream_index],
              record_count, result_params.filename,
              result_params.stream_specifications, result_params.stream_index);
        } else {
          WriteResults(
              data_manager,
              allocated_memory_blocks.at(node_name)[result_params.stream_index],
              record_count, result_params.filename,
              result_params.stream_specifications, result_params.stream_index);
        }
      }
    }
  }
}

void QueryManager::FreeMemoryBlocks(
    fpga_managing::MemoryManager& memory_manager,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        input_memory_blocks,
    std::map<std::string,
             std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>& input_stream_sizes,
    std::map<std::string, std::vector<RecordSizeAndCount>>& output_stream_sizes,
    std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links,
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        scheduled_nodes) {
  std::vector<std::string> removable_vectors;
  for (const auto& node : scheduled_nodes) {
    for (auto& memory_block : input_memory_blocks[node->node_name]) {
      if (memory_block) {
        memory_manager.FreeMemoryBlock(std::move(memory_block));
      }
      memory_block = nullptr;
    }
    input_stream_sizes.erase(node->node_name);
    input_memory_blocks.erase(node->node_name);
  }

  for (const auto& [node_name, data_mapping] : reuse_links) {
    for (const auto& [output_stream_index, targe_input_streams] :
         data_mapping) {
      if (targe_input_streams.size() == 1) {
        auto target_node_name = targe_input_streams.at(0).first;
        auto target_stream_index = targe_input_streams.at(0).second;
        removable_vectors.erase(
            std::remove(removable_vectors.begin(), removable_vectors.end(),
                        target_node_name),
            removable_vectors.end());
        input_memory_blocks[target_node_name][target_stream_index] =
            std::move(output_memory_blocks[node_name][output_stream_index]);
        output_memory_blocks[node_name][output_stream_index] = nullptr;
        input_stream_sizes[target_node_name][target_stream_index] =
            output_stream_sizes[node_name][output_stream_index];
      } else {
        for (const auto& [target_node_name, target_stream_index] :
             targe_input_streams) {
          removable_vectors.erase(
              std::remove(removable_vectors.begin(), removable_vectors.end(),
                          target_node_name),
              removable_vectors.end());
          input_memory_blocks[target_node_name][target_stream_index] =
              std::move(memory_manager.GetAvailableMemoryBlock());
          CopyMemoryData(
              output_stream_sizes[node_name][output_stream_index].first *
                  output_stream_sizes[node_name][output_stream_index].second,
              output_memory_blocks[node_name][output_stream_index],
              input_memory_blocks[target_node_name][target_stream_index]);
          input_stream_sizes[target_node_name][target_stream_index] =
              output_stream_sizes[node_name][output_stream_index];
        }
      }
    }
  }

  for (auto& [node_name, memory_block_vector] : output_memory_blocks) {
    for (auto& memory_block : memory_block_vector) {
      if (memory_block) {
        memory_manager.FreeMemoryBlock(std::move(memory_block));
      }
    }
  }

  output_memory_blocks.clear();
  output_stream_sizes.clear();
}

void QueryManager::CheckResults(
    const DataManager& data_manager,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& memory_device,
    int row_count, std::string filename,
    const std::vector<std::vector<int>>& node_parameters, int stream_index) {
  auto expected_table = TableManager::ReadTableFromFile(
      data_manager, node_parameters, stream_index, std::move(filename));
  auto resulting_table = TableManager::ReadTableFromMemory(
      data_manager, node_parameters, stream_index, memory_device, row_count);
  CheckTableData(expected_table, resulting_table);
}
void QueryManager::WriteResults(
    const DataManager& data_manager,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>& memory_device,
    int row_count, std::string filename,
    const std::vector<std::vector<int>>& node_parameters, int stream_index) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  auto resulting_table = TableManager::ReadTableFromMemory(
      data_manager, node_parameters, stream_index, memory_device, row_count);
  TableManager::WriteResultTableFile(resulting_table, std::move(filename));

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  Log(LogLevel::kInfo,
      "Write result data time = " +
          std::to_string(
              std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[ms]");
}
void QueryManager::CopyMemoryData(
    int table_size,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>&
        source_memory_device,
    const std::unique_ptr<fpga_managing::MemoryBlockInterface>&
        target_memory_device) {
  volatile uint32_t* source = source_memory_device->GetVirtualAddress();
  volatile uint32_t* target = target_memory_device->GetVirtualAddress();
  for (int i = 0; i < table_size; i++) {
    target[i] = source[i];
  }
}

auto QueryManager::GetCurrentLinks(
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
        scheduled_nodes,
    const std::map<std::string, std::map<int, MemoryReuseTargets>>&
        all_reuse_links)
    -> std::map<std::string, std::map<int, MemoryReuseTargets>> {
  std::map<std::string, std::map<int, MemoryReuseTargets>> current_links;
  for (const auto& [node_name, data_mapping] : all_reuse_links) {
    auto search = std::find_if(
        scheduled_nodes.begin(), scheduled_nodes.end(),
        [&](const auto& node) { return node->node_name == node_name; });
    if (search != scheduled_nodes.end()) {
      current_links.insert({node_name, data_mapping});
    }
  }
  return current_links;
}

void QueryManager::RunQueries(
    std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
        starting_query_nodes,
    const Config& config) {
  Log(LogLevel::kTrace, "Starting up!");
  data_managing::DataManager data_manager(config.data_sizes, config.separator);
  fpga_managing::MemoryManager memory_manager;
  fpga_managing::FPGAManager fpga_manager(&memory_manager);

  std::map<std::string, std::map<int, MemoryReuseTargets>> all_reuse_links;

  auto query_node_runs_queue = NodeScheduler::FindAcceleratedQueryNodeSets(
      std::move(starting_query_nodes), config.accelerator_library,
      config.module_library, all_reuse_links);
  Log(LogLevel::kTrace, "Scheduling done!");

  std::map<std::string,
           std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>
      input_memory_blocks;
  std::map<std::string,
           std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>
      output_memory_blocks;
  std::map<std::string, std::vector<RecordSizeAndCount>> input_stream_sizes;
  std::map<std::string, std::vector<RecordSizeAndCount>> output_stream_sizes;

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

    auto current_run_links =
        GetCurrentLinks(executable_query_nodes, all_reuse_links);

    InitialiseVectorSizes(executable_query_nodes, input_memory_blocks,
                          output_memory_blocks, input_stream_sizes,
                          output_stream_sizes);

    IDManager::AllocateStreamIDs(CreateReferenceVector(executable_query_nodes),
                                 input_ids, output_ids);

    for (const auto& node : executable_query_nodes) {
      AllocateOutputMemoryBlocks(memory_manager, data_manager,
                                 output_memory_blocks[node->node_name], *node,
                                 output_stream_sizes[node->node_name]);
      AllocateInputMemoryBlocks(
          memory_manager, data_manager, input_memory_blocks[node->node_name],
          *node, output_stream_sizes, input_stream_sizes[node->node_name]);

      auto input_params =
          CreateStreamParams(input_ids[node->node_name],
                             node->operation_parameters.input_stream_parameters,
                             input_memory_blocks[node->node_name],
                             input_stream_sizes[node->node_name]);
      auto output_params = CreateStreamParams(
          output_ids[node->node_name],
          node->operation_parameters.output_stream_parameters,
          output_memory_blocks[node->node_name],
          output_stream_sizes[node->node_name]);

      // If input stream sizes are not checked in the scheduler since previous
      // node stream sizes are unknown. For futher scheduling improvements the
      // scheduling should be redone after each run possibly?
      ElasticModuleChecker::CheckElasticityNeeds(
          input_params, node->operation_type,
          node->operation_parameters.operation_parameters);
      query_nodes.push_back({std::move(input_params), std::move(output_params),
                             node->operation_type, node->module_location,
                             node->operation_parameters.operation_parameters});
      StoreStreamResultPrameters(result_parameters, output_ids[node->node_name],
                                 *node, output_memory_blocks[node->node_name]);
    }

      std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    Log(LogLevel::kTrace, "Setup query!");
    fpga_manager.SetupQueryAcceleration(query_nodes);
    Log(LogLevel::kTrace, "Running query!");
    auto result_sizes = fpga_manager.RunQueryAcceleration();
    Log(LogLevel::kTrace, "Query done!");

        std::chrono::steady_clock::time_point end =
        std::chrono::steady_clock::now();
    Log(LogLevel::kInfo,
        "Init and run time = " +
            std::to_string(
                std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                      begin)
                    .count()) +
            "[ms]");

    ProcessResults(data_manager, result_sizes, result_parameters,
                   output_memory_blocks, output_stream_sizes);
    FreeMemoryBlocks(memory_manager, input_memory_blocks, output_memory_blocks,
                     input_stream_sizes, output_stream_sizes, current_run_links,
                     executable_query_nodes);
  }
}