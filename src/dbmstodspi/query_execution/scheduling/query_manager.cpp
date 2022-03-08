/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "query_manager.hpp"

#include <algorithm>
#include <stdexcept>

#include <iostream>

#include "bitstream_config_helper.hpp"
#include "elastic_module_checker.hpp"
#include "fpga_manager.hpp"
#include "id_manager.hpp"
#include "logger.hpp"
#include "node_scheduler_interface.hpp"
#include "query_acceleration_constants.hpp"
#include "run_linker.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"
#include "table_manager.hpp"
#include "util.hpp"

using orkhestrafs::core_interfaces::table_data::SortedSequence;
using orkhestrafs::dbmstodspi::BitstreamConfigHelper;
using orkhestrafs::dbmstodspi::QueryManager;
using orkhestrafs::dbmstodspi::RunLinker;
using orkhestrafs::dbmstodspi::StreamDataParameters;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::query_acceleration_constants::kIOStreamParamDefs;
using orkhestrafs::dbmstodspi::util::CreateReferenceVector;

auto QueryManager::GetCurrentLinks(
    const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
    const std::map<std::string, std::map<int, MemoryReuseTargets>>&
        all_reuse_links)
    -> std::map<std::string, std::map<int, MemoryReuseTargets>> {
  std::map<std::string, std::map<int, MemoryReuseTargets>> current_links;
  for (const auto& [node_name, data_mapping] : all_reuse_links) {
    auto search = std::find_if(
        current_query_nodes.begin(), current_query_nodes.end(),
        [&](const auto& node) { return node->node_name == node_name; });
    if (search != current_query_nodes.end()) {
      current_links.insert({node_name, data_mapping});
    }
  }
  return current_links;
}

void QueryManager::InitialiseMemoryBlockVector(
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
        memory_blocks,
    int stream_count, const std::string& node_name) {
  std::vector<std::unique_ptr<MemoryBlockInterface>> empty_vector(stream_count);
  std::fill(empty_vector.begin(), empty_vector.end(), nullptr);
  memory_blocks.insert({node_name, std::move(empty_vector)});
}

void QueryManager::InitialiseStreamSizeVector(
    std::map<std::string, std::vector<RecordSizeAndCount>>& stream_sizes,
    int stream_count, const std::string& node_name) {
  std::vector<RecordSizeAndCount> empty_vector(stream_count);
  std::fill(empty_vector.begin(), empty_vector.end(), std::make_pair(0, 0));
  stream_sizes.insert({node_name, std::move(empty_vector)});
}

// Create map with correct amount of elements locations and data reuse links
void QueryManager::InitialiseVectorSizes(
    const std::vector<std::shared_ptr<QueryNode>>& scheduled_nodes,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
        input_memory_blocks,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
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

auto QueryManager::GetRecordSizeFromParameters(
    const DataManagerInterface* data_manager,
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

void QueryManager::AllocateOutputMemoryBlocks(
    MemoryManagerInterface* memory_manager,
    const DataManagerInterface* data_manager,
    std::vector<std::unique_ptr<MemoryBlockInterface>>& output_memory_blocks,
    const QueryNode& node,
    std::vector<RecordSizeAndCount>& output_stream_sizes) {
  for (int stream_index = 0; stream_index < node.next_nodes.size();
       stream_index++) {
    if (!node.next_nodes[stream_index]) {
      output_memory_blocks[stream_index] =
          std::move(memory_manager->GetAvailableMemoryBlock());
      // Uncomment to check overwriting.
      /*for (int i = 0; i < 100000; i++) {
        output_memory_blocks[stream_index]->GetVirtualAddress()[i] = -1;
      }*/
    }
    output_stream_sizes[stream_index].first = GetRecordSizeFromParameters(
        data_manager, node.operation_parameters.output_stream_parameters,
        stream_index);
  }
}
void QueryManager::AllocateInputMemoryBlocks(
    MemoryManagerInterface* memory_manager,
    const DataManagerInterface* data_manager,
    std::vector<std::unique_ptr<MemoryBlockInterface>>& input_memory_blocks,
    const QueryNode& node,
    const std::map<std::string, std::vector<RecordSizeAndCount>>&
        output_stream_sizes,
    std::vector<RecordSizeAndCount>& input_stream_sizes) {
  for (int stream_index = 0; stream_index < node.previous_nodes.size();
       stream_index++) {
    auto observed_node = node.previous_nodes[stream_index].lock();
    if (!observed_node && !input_memory_blocks[stream_index]) {
      input_memory_blocks[stream_index] =
          std::move(memory_manager->GetAvailableMemoryBlock());
      std::chrono::steady_clock::time_point begin =
          std::chrono::steady_clock::now();

      input_stream_sizes[stream_index] = TableManager::WriteDataToMemory(
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
    bool is_input, const QueryNode& node,
    AcceleratorLibraryInterface* accelerator_library,
    const std::vector<int>& stream_ids,
    const std::vector<std::unique_ptr<MemoryBlockInterface>>&
        allocated_memory_blocks,
    const std::vector<RecordSizeAndCount>& stream_sizes)
    -> std::vector<StreamDataParameters> {
  auto node_parameters =
      (is_input) ? node.operation_parameters.input_stream_parameters
                 : node.operation_parameters.output_stream_parameters;

  std::vector<StreamDataParameters> parameters_for_acceleration;

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

    auto [channel_count, records_per_channel] =
        accelerator_library->GetMultiChannelParams(
            is_input, stream_index, node.operation_type,
            node.operation_parameters.operation_parameters);
    StreamDataParameters current_stream_parameters = {
        stream_ids[stream_index],
        stream_sizes[stream_index].first,
        stream_sizes[stream_index].second,
        physical_address_ptr,
        node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                           kIOStreamParamDefs.kProjectionOffset),
        chunk_count,
        channel_count,
        records_per_channel};

    parameters_for_acceleration.push_back(current_stream_parameters);
  }
  return parameters_for_acceleration;
}

void QueryManager::StoreStreamResultParameters(
    std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::vector<int>& stream_ids, const QueryNode& node,
    const std::vector<std::unique_ptr<MemoryBlockInterface>>&
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

auto QueryManager::SetupAccelerationNodesForExecution(
    DataManagerInterface* data_manager, MemoryManagerInterface* memory_manager,
    AcceleratorLibraryInterface* accelerator_library,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
        input_memory_blocks,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>& input_stream_sizes,
    std::map<std::string, std::vector<RecordSizeAndCount>>& output_stream_sizes,
    const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes)
    -> std::pair<std::vector<AcceleratedQueryNode>,
                 std::map<std::string, std::vector<StreamResultParameters>>> {
  std::map<std::string, std::vector<StreamResultParameters>> result_parameters;
  std::vector<AcceleratedQueryNode> query_nodes;

  std::map<std::string, std::vector<int>> output_ids;
  std::map<std::string, std::vector<int>> input_ids;

  InitialiseVectorSizes(current_query_nodes, input_memory_blocks,
                        output_memory_blocks, input_stream_sizes,
                        output_stream_sizes);

  IDManager::AllocateStreamIDs(CreateReferenceVector(current_query_nodes),
                               input_ids, output_ids);

  for (const auto& node : current_query_nodes) {
    AllocateOutputMemoryBlocks(memory_manager, data_manager,
                               output_memory_blocks[node->node_name], *node,
                               output_stream_sizes[node->node_name]);
    AllocateInputMemoryBlocks(
        memory_manager, data_manager, input_memory_blocks[node->node_name],
        *node, output_stream_sizes, input_stream_sizes[node->node_name]);

    auto input_params = CreateStreamParams(true, *node, accelerator_library,
                                           input_ids[node->node_name],
                                           input_memory_blocks[node->node_name],
                                           input_stream_sizes[node->node_name]);
    auto output_params = CreateStreamParams(
        false, *node, accelerator_library, output_ids[node->node_name],
        output_memory_blocks[node->node_name],
        output_stream_sizes[node->node_name]);

    AddQueryNodes(query_nodes, std::move(input_params),
                  std::move(output_params), *node);

    StoreStreamResultParameters(result_parameters, output_ids[node->node_name],
                                *node, output_memory_blocks[node->node_name]);
  }

  std::sort(query_nodes.begin(), query_nodes.end(),
            [](AcceleratedQueryNode const& lhs,
               AcceleratedQueryNode const& rhs) -> bool {
              return lhs.operation_module_location <
                     rhs.operation_module_location;
            });

  return {query_nodes, result_parameters};
}

void QueryManager::LoadNextBitstreamIfNew(
    MemoryManagerInterface* memory_manager, std::string bitstream_file_name,
    Config config) {
  return memory_manager->LoadBitstreamIfNew(
      bitstream_file_name,
      config.required_memory_space.at(bitstream_file_name));
}

void QueryManager::LoadInitialStaticBitstream(
    MemoryManagerInterface* memory_manager) {
  memory_manager->LoadStatic();
}

void QueryManager::LoadEmptyRoutingPRRegion(
    MemoryManagerInterface* memory_manager,
    AcceleratorLibraryInterface& driver_library) {
  std::vector<std::string> empty_pr_region = {
      "RT_95.bin", "RT_92.bin", "RT_89.bin", "RT_86.bin", "RT_83.bin",
      "RT_80.bin", "RT_77.bin", "RT_74.bin", "RT_71.bin", "RT_68.bin",
      "RT_65.bin", "RT_62.bin", "RT_59.bin", "RT_56.bin", "RT_53.bin",
      "RT_50.bin", "RT_47.bin", "RT_44.bin", "RT_41.bin", "RT_38.bin",
      "RT_35.bin", "RT_32.bin", "RT_29.bin", "RT_26.bin", "RT_23.bin",
      "RT_20.bin", "RT_17.bin", "RT_14.bin", "RT_11.bin", "RT_8.bin",
      "RT_5.bin",  "TAA_2.bin"};
  LoadPRBitstreams(memory_manager, empty_pr_region, driver_library);
}

void QueryManager::LoadPRBitstreams(
    MemoryManagerInterface* memory_manager,
    const std::vector<std::string>& bitstream_names,
    AcceleratorLibraryInterface& driver_library) {
  auto dma_module = driver_library.GetDMAModule();
  memory_manager->LoadPartialBitstream(bitstream_names, *dma_module);
}

void QueryManager::BenchmarkScheduling(
    const std::vector<std::string>& first_node_names,
    std::vector<std::string>& starting_nodes,
    std::vector<std::string>& processed_nodes,
    std::map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& tables,
    AcceleratorLibraryInterface& drivers, const Config& config,
    NodeSchedulerInterface& node_scheduler,
    std::vector<ScheduledModule>& current_configuration) {
  node_scheduler.BenchmarkScheduling(
      first_node_names, starting_nodes, processed_nodes, graph, drivers, tables,
      current_configuration, config, benchmark_stats_);
}

void QueryManager::PrintBenchmarkStats() {
  for (const auto& [stats_key,value] : benchmark_stats_) {
    std::cout << stats_key << ": " << value << std::endl;
  }
}

auto QueryManager::ScheduleNextSetOfNodes(
    std::vector<std::shared_ptr<QueryNode>>& query_nodes,
    const std::vector<std::string>& first_node_names,
    std::vector<std::string>& starting_nodes,
    std::vector<std::string>& processed_nodes,
    std::map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& tables,
    AcceleratorLibraryInterface& drivers, const Config& config,
    NodeSchedulerInterface& node_scheduler,
    std::map<std::string, std::map<int, MemoryReuseTargets>>& all_reuse_links,
    const std::vector<ScheduledModule>& current_configuration)
    -> std::queue<std::pair<std::vector<ScheduledModule>,
                            std::vector<std::shared_ptr<QueryNode>>>> {
  auto current_queue = node_scheduler.GetNextSetOfRuns(
      query_nodes, first_node_names, starting_nodes, processed_nodes, graph,
      drivers, tables, current_configuration, config);
  return RunLinker::LinkPeripheralNodesFromGivenRuns(current_queue,
                                                     all_reuse_links);
}

auto QueryManager::GetPRBitstreamsToLoadWithPassthroughModules(
    std::vector<ScheduledModule>& current_config,
    const std::vector<ScheduledModule>& next_config, int column_count)
    -> std::pair<std::vector<std::string>,
                 std::vector<std::pair<QueryOperationType, bool>>> {
  std::vector<int> written_frames(column_count, false);
  auto [reduced_next_config, reduced_current_config] =
      BitstreamConfigHelper::GetConfigCompliments(next_config, current_config);

  // Find out which frames need writing to.
  for (const auto& module : reduced_current_config) {
    for (int column_i = module.position.first;
         column_i < module.position.second + 1; column_i++) {
      written_frames[column_i] = true;
    }
  }
  std::vector<std::string> required_bitstreams;
  for (const auto& module : reduced_next_config) {
    for (int column_i = module.position.first;
         column_i < module.position.second + 1; column_i++) {
      written_frames[column_i] = false;
    }
    required_bitstreams.push_back(module.bitstream);
  }

  // Hardoded for now.
  std::vector<std::string> routing_bitstreams = {
      "RT_95.bin", "RT_92.bin", "RT_89.bin", "RT_86.bin", "RT_83.bin",
      "RT_80.bin", "RT_77.bin", "RT_74.bin", "RT_71.bin", "RT_68.bin",
      "RT_65.bin", "RT_62.bin", "RT_59.bin", "RT_56.bin", "RT_53.bin",
      "RT_50.bin", "RT_47.bin", "RT_44.bin", "RT_41.bin", "RT_38.bin",
      "RT_35.bin", "RT_32.bin", "RT_29.bin", "RT_26.bin", "RT_23.bin",
      "RT_20.bin", "RT_17.bin", "RT_14.bin", "RT_11.bin", "RT_8.bin",
      "RT_5.bin"};

  for (int column_i = 0; column_i < column_count; column_i++) {
    if (written_frames.at(column_i)) {
      required_bitstreams.push_back(routing_bitstreams.at(column_i));
    }
  }

  auto left_over_config = BitstreamConfigHelper::GetResultingConfig(
      next_config, reduced_next_config, reduced_current_config);
  std::sort(left_over_config.begin(), left_over_config.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs.position.first < rhs.position.first;
            });

  std::vector<std::pair<QueryOperationType, bool>> passthrough_modules;
  for (const auto& module : left_over_config) {
    if (std::find(next_config.begin(), next_config.end(), module) ==
        next_config.end()) {
      passthrough_modules.push_back({module.operation_type, false});
    } else {
      passthrough_modules.push_back({module.operation_type, true});
    }
  }

  current_config = left_over_config;

  return {required_bitstreams, passthrough_modules};
}

// auto QueryManager::IsRunValid(std::vector<AcceleratedQueryNode> current_run)
//    -> bool {
//  for (const auto& node : current_run) {
//    if (!ElasticModuleChecker::IsRunValid(node.input_streams,
//                                          node.operation_type,
//                                          node.operation_parameters)) {
//      return false;
//    }
//  }
//  return true;
//}

void QueryManager::CheckTableData(const DataManagerInterface* data_manager,
                                  const TableData& expected_table,
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
    data_manager->PrintTableData(resulting_table);
  }
}

// This checking needs to get redone to not use the CheckTableData method and
// instead compare memory blocks. For extra debugging (i.e., finding the lines
// which are different) a separate debug method should be added.
void QueryManager::CheckResults(
    const DataManagerInterface* data_manager,
    const std::unique_ptr<MemoryBlockInterface>& memory_device, int row_count,
    const std::string& filename,
    const std::vector<std::vector<int>>& node_parameters, int stream_index) {
  auto expected_table = TableManager::ReadTableFromFile(
      data_manager, node_parameters, stream_index, filename);
  auto resulting_table = TableManager::ReadTableFromMemory(
      data_manager, node_parameters, stream_index, memory_device, row_count);
  CheckTableData(data_manager, expected_table, resulting_table);
}

void QueryManager::WriteResults(
    const DataManagerInterface* data_manager,
    const std::unique_ptr<MemoryBlockInterface>& memory_device, int row_count,
    const std::string& filename,
    const std::vector<std::vector<int>>& node_parameters, int stream_index) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  auto resulting_table = TableManager::ReadTableFromMemory(
      data_manager, node_parameters, stream_index, memory_device, row_count);
  TableManager::WriteResultTableFile(data_manager, resulting_table, filename);

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
    const std::unique_ptr<MemoryBlockInterface>& source_memory_device,
    const std::unique_ptr<MemoryBlockInterface>& target_memory_device) {
  volatile uint32_t* source = source_memory_device->GetVirtualAddress();
  volatile uint32_t* target = target_memory_device->GetVirtualAddress();
  for (int i = 0; i < table_size; i++) {
    target[i] = source[i];
  }
}

void QueryManager::ProcessResults(
    const DataManagerInterface* data_manager,
    const std::array<int, query_acceleration_constants::kMaxIOStreamCount>
        result_sizes,
    const std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::map<std::string,
                   std::vector<std::unique_ptr<MemoryBlockInterface>>>&
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
    MemoryManagerInterface* memory_manager,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
        input_memory_blocks,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>& input_stream_sizes,
    std::map<std::string, std::vector<RecordSizeAndCount>>& output_stream_sizes,
    const std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links,
    const std::vector<std::string>& scheduled_node_names) {
  std::vector<std::string> removable_vectors;
  for (const auto& name : scheduled_node_names) {
    for (auto& memory_block : input_memory_blocks[name]) {
      if (memory_block) {
        memory_manager->FreeMemoryBlock(std::move(memory_block));
      }
      memory_block = nullptr;
    }
    input_stream_sizes.erase(name);
    input_memory_blocks.erase(name);
  }

  for (const auto& [node_name, data_mapping] : reuse_links) {
    for (const auto& [output_stream_index, target_input_streams] :
         data_mapping) {
      if (target_input_streams.size() == 1) {
        auto target_node_name = target_input_streams.at(0).first;
        auto target_stream_index = target_input_streams.at(0).second;
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
             target_input_streams) {
          removable_vectors.erase(
              std::remove(removable_vectors.begin(), removable_vectors.end(),
                          target_node_name),
              removable_vectors.end());
          input_memory_blocks[target_node_name][target_stream_index] =
              std::move(memory_manager->GetAvailableMemoryBlock());
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
        memory_manager->FreeMemoryBlock(std::move(memory_block));
      }
    }
  }

  output_memory_blocks.clear();
  output_stream_sizes.clear();
}

void QueryManager::ExecuteAndProcessResults(
    FPGAManagerInterface* fpga_manager,
    const DataManagerInterface* data_manager,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>&
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>& output_stream_sizes,
    const std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::vector<AcceleratedQueryNode>& execution_query_nodes,
    std::map<std::string, TableMetadata>& scheduling_table_data,
    const std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links,
    std::map<std::string, SchedulingQueryNode>& scheduling_graph) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  Log(LogLevel::kTrace, "Setup query!");
  fpga_manager->SetupQueryAcceleration(execution_query_nodes);
  Log(LogLevel::kTrace, "Running query!");
  auto result_sizes = fpga_manager->RunQueryAcceleration();
  Log(LogLevel::kTrace, "Query done!");

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  Log(LogLevel::kInfo,
      "Init and run time = " +
          std::to_string(
              std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
                  .count()) +
          "[microseconds]");

  ProcessResults(data_manager, result_sizes, result_parameters,
                 output_memory_blocks, output_stream_sizes);

  UpdateTableData(result_parameters, output_stream_sizes, scheduling_table_data,
                  reuse_links, scheduling_graph);
}

void QueryManager::UpdateTableData(
    const std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::map<std::string, std::vector<RecordSizeAndCount>>&
        output_stream_sizes,
    std::map<std::string, TableMetadata>& scheduling_table_data,
    const std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links,
    std::map<std::string, SchedulingQueryNode>& scheduling_graph) {
  // TODO: This needs to get tested!!!
  // Also this is assuming that there always is a link. There might not be.
  for (const auto& [node_name, stream_parameters] : result_parameters) {
    for (int stream_index = 0; stream_index < stream_parameters.size();
         stream_index++) {
      auto filename = stream_parameters.at(stream_index).filename;
      TableMetadata current_data = {
          output_stream_sizes.at(node_name).at(stream_index).first,
          output_stream_sizes.at(node_name).at(stream_index).second,
          {}};

      // Just in case another table has to be created. Untested as the sorted
      // status is a mess!
      if (reuse_links.find(node_name) != reuse_links.end()) {
        for (const auto& target : reuse_links.at(node_name).at(stream_index)) {
          if (scheduling_graph.find(target.first) != scheduling_graph.end()) {
            auto target_filename =
                scheduling_graph.at(target.first).data_tables.at(target.second);
            if (filename.empty()) {
              scheduling_table_data.at(target_filename).record_count =
                  current_data.record_count;
              scheduling_table_data.at(target_filename).record_size =
                  current_data.record_size;
            } else {
              if (const auto& [it, inserted] =
                      scheduling_table_data.emplace(filename, current_data);
                  !inserted) {
                scheduling_table_data.at(filename).record_count =
                    current_data.record_count;
                scheduling_table_data.at(filename).record_size =
                    current_data.record_size;
              } else {
                scheduling_graph.at(target.first)
                    .data_tables.at(target.second) = filename;
              }
            }
            auto next_node_data = scheduling_graph.at(target.first);
            CropSortedStatus(scheduling_table_data,
                             next_node_data.data_tables.at(target.second));
            auto next_node_table = scheduling_table_data.at(
                next_node_data.data_tables.at(target.second));
          }
        }
      }
    }
  }
}

void QueryManager::CropSortedStatus(
    std::map<std::string, TableMetadata>& scheduling_table_data,
    std::string filename) {
  auto current_data = scheduling_table_data.at(filename);
  if (!current_data.sorted_status.empty()) {
    if (current_data.sorted_status.back().start_position +
            current_data.sorted_status.back().length >
        current_data.record_count) {
      std::vector<SortedSequence> cropped_sequences;
      for (const auto& sequence : current_data.sorted_status) {
        if (sequence.start_position + sequence.length <=
            current_data.record_count) {
          cropped_sequences.push_back(sequence);
        } else if (sequence.start_position < current_data.record_count) {
          cropped_sequences.push_back(
              {sequence.start_position,
               current_data.record_count - sequence.start_position});
        }
      }
      scheduling_table_data.at(filename).sorted_status = cropped_sequences;
    }
  }
}

void QueryManager::AddQueryNodes(
    std::vector<AcceleratedQueryNode>& query_nodes_vector,
    std::vector<StreamDataParameters>&& input_params,
    std::vector<StreamDataParameters>&& output_params, const QueryNode& node) {
  auto sorted_module_locations = node.module_locations;
  std::sort(sorted_module_locations.begin(), sorted_module_locations.end());
  auto no_io_input_params = input_params;
  for (auto& stream_param : no_io_input_params) {
    stream_param.physical_address = nullptr;
  }
  auto no_io_output_params = output_params;
  for (auto& stream_param : no_io_output_params) {
    stream_param.physical_address = nullptr;
  }
  if (sorted_module_locations.size() == 1) {
    query_nodes_vector.push_back(
        {std::move(input_params),
         std::move(output_params),
         node.operation_type,
         sorted_module_locations.at(0),
         {},
         node.operation_parameters.operation_parameters});
  } else if (node.operation_parameters.operation_parameters.empty()) {
    // Multiple composed modules
    for (int i = 0; i < sorted_module_locations.size(); i++) {
      auto current_input = (i == 0) ? input_params : no_io_input_params;
      auto current_output = (i == sorted_module_locations.size() - 1)
                                ? output_params
                                : no_io_output_params;
      query_nodes_vector.push_back(
          {current_input, current_output, node.operation_type,
           sorted_module_locations.at(i), sorted_module_locations,
           node.operation_parameters.operation_parameters});
    }
  } else {
    // Multiple resource elastic modules.
    auto all_module_params = node.operation_parameters.operation_parameters;
    if (all_module_params.at(0).at(0) != sorted_module_locations.size()) {
      throw std::runtime_error("Wrong parameters given!");
    }
    int offset = all_module_params.at(0).at(1);
    for (int i = 0; i < sorted_module_locations.size(); i++) {
      auto current_input = (i == 0) ? input_params : no_io_input_params;
      auto current_output = (i == sorted_module_locations.size() - 1)
                                ? output_params
                                : no_io_output_params;
      query_nodes_vector.push_back(
          {current_input,
           current_output,
           node.operation_type,
           sorted_module_locations.at(i),
           sorted_module_locations,
           {all_module_params.begin() + 1 + offset * i,
            all_module_params.begin() + 1 + offset * (i + 1)}});
    }
  }
}
