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
#include <chrono>
#include <iostream>
#include <stdexcept>

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

using orkhestrafs::dbmstodspi::BitstreamConfigHelper;
using orkhestrafs::dbmstodspi::QueryManager;
using orkhestrafs::dbmstodspi::RunLinker;
using orkhestrafs::dbmstodspi::StreamDataParameters;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::query_acceleration_constants::kIOStreamParamDefs;
using orkhestrafs::dbmstodspi::util::CreateReferenceVector;

auto QueryManager::GetCurrentLinks(
    std::queue<std::map<std::string, std::map<int, MemoryReuseTargets>>>&
        all_reuse_links)
    -> std::map<std::string, std::map<int, MemoryReuseTargets>> {
  auto cur_links = all_reuse_links.front();
  all_reuse_links.pop();
  return cur_links;
}

void QueryManager::InitialiseMemoryBlockVector(
    std::map<std::string, std::vector<MemoryBlockInterface*>>& memory_blocks,
    int stream_count, const std::string& node_name) {
  std::vector<MemoryBlockInterface*> empty_vector(stream_count);
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
    std::map<std::string, std::vector<MemoryBlockInterface*>>&
        input_memory_blocks,
    std::map<std::string, std::vector<MemoryBlockInterface*>>&
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>& input_stream_sizes,
    std::map<std::string, std::vector<RecordSizeAndCount>>&
        output_stream_sizes) {
  //  for (const auto& node : scheduled_nodes) {
  //    // Input could be defined from previous runs
  //    if (input_memory_blocks.find(node->node_name) ==
  //        input_memory_blocks.end()) {
  //      InitialiseMemoryBlockVector(input_memory_blocks,
  //                                  node->previous_nodes.size(),
  //                                  node->node_name);
  //      InitialiseStreamSizeVector(input_stream_sizes,
  //                                 node->previous_nodes.size(),
  //                                 node->node_name);
  //    }
  //
  //    InitialiseMemoryBlockVector(output_memory_blocks,
  //    node->next_nodes.size(),
  //                                node->node_name);
  //    InitialiseStreamSizeVector(output_stream_sizes,
  //    node->previous_nodes.size(),
  //                               node->node_name);
  //
  //    for (auto& output_node : node->next_nodes) {
  //      if (output_node) {
  //        if (input_memory_blocks.find(output_node->node_name) ==
  //            input_memory_blocks.end()) {
  //          InitialiseMemoryBlockVector(input_memory_blocks,
  //                                      output_node->previous_nodes.size(),
  //                                      output_node->node_name);
  //          InitialiseStreamSizeVector(input_stream_sizes,
  //                                     output_node->previous_nodes.size(),
  //                                     output_node->node_name);
  //        }
  //        if (std::find(scheduled_nodes.begin(), scheduled_nodes.end(),
  //                      output_node) == scheduled_nodes.end()) {
  //          output_node = nullptr;
  //        }
  //      }
  //    }
  //  }
}

auto QueryManager::GetRecordSizeFromParameters(
    const DataManagerInterface* data_manager,
    const std::vector<std::vector<int>>& node_parameters,
    int stream_index) const -> int {
  auto column_defs_vector = TableManager::GetColumnDefsVector(
      data_manager, node_parameters, stream_index);

  int record_size = 0;
  for (const auto& column_type : column_defs_vector) {
    record_size += column_type.second;
  }
  return record_size;
}

void QueryManager::AllocateOutputMemoryBlocks(
    MemoryManagerInterface* memory_manager, const NodeRunData& run_data,
    std::unordered_map<std::string, MemoryBlockInterface*>&
        table_memory_blocks) {
  for (const auto& output_table : run_data.output_data_definition_files) {
    if (!output_table.empty() &&
        table_memory_blocks.find(output_table) == table_memory_blocks.end()) {
      table_memory_blocks[output_table] =
          std::move(memory_manager->GetAvailableMemoryBlock());
      // Uncomment to check overwriting.
      /*for (int i = 0; i < 100000; i++) {
        output_memory_blocks[stream_index]->GetVirtualAddress()[i] = -1;
      }*/
    }
  }
}
void QueryManager::AllocateInputMemoryBlocks(
    MemoryManagerInterface* memory_manager,
    const DataManagerInterface* data_manager, const NodeRunData& run_data,
    const std::vector<std::vector<int>>& input_stream_parameters,
    const std::map<std::string, TableMetadata>& current_tables_metadata,
    std::unordered_map<std::string, MemoryBlockInterface*>&
        table_memory_blocks) {
  for (int stream_index = 0;
       stream_index < run_data.input_data_definition_files.size();
       stream_index++) {
    const auto& input_table =
        run_data.input_data_definition_files.at(stream_index);
    if (!input_table.empty() &&
        table_memory_blocks.find(input_table) == table_memory_blocks.end()) {
      table_memory_blocks[input_table] =
          std::move(memory_manager->GetAvailableMemoryBlock());
      std::chrono::steady_clock::time_point begin =
          std::chrono::steady_clock::now();

      auto [record_size, record_count] = TableManager::WriteDataToMemory(
          data_manager, input_stream_parameters, stream_index,
          table_memory_blocks[input_table], input_table);

      std::chrono::steady_clock::time_point end =
          std::chrono::steady_clock::now();
      std::cout << "FS TO MEMORY WRITE:"
                << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                         begin)
                       .count()
                << std::endl;
      Log(LogLevel::kInfo,
          "Read data time = " +
              std::to_string(
                  std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                        begin)
                      .count()) +
              "[ms]");
      if (record_size != current_tables_metadata.at(input_table).record_size ||
          record_count !=
              current_tables_metadata.at(input_table).record_count) {
        throw std::runtime_error(
            "Mismatch of table sizes after reading table data!");
      }
    }
  }
}

auto QueryManager::CreateStreamParams(
    bool is_input, const QueryNode* node, const std::vector<int>& stream_ids,
    const NodeRunData& run_data,
    const std::map<std::string, TableMetadata>& current_tables_metadata,
    const std::unordered_map<std::string, MemoryBlockInterface*>&
        table_memory_blocks) -> std::vector<StreamDataParameters> {
  auto node_parameters =
      (is_input) ? node->given_operation_parameters.input_stream_parameters
                 : node->given_operation_parameters.output_stream_parameters;

  std::vector<StreamDataParameters> parameters_for_acceleration;

  const int table_offset = 0;
  const int offset_offset = 1;
  const int count_offset = 2;

  for (int stream_index = 0; stream_index < stream_ids.size(); stream_index++) {
    const auto& table_name =
        run_data.input_data_definition_files.at(stream_index);
    std::map<volatile uint32_t*, std::vector<int>> physical_addresses_map;
    int virtual_channel_count = -1;
    if (is_input && !table_name.empty()) {
      if (node->operation_type == QueryOperationType::kMergeSort) {
        std::vector<int> current_sequences;
        int current_table_index =
            run_data.operation_parameters.front().at(table_offset);
        int current_offset =
            run_data.operation_parameters.front().at(offset_offset);
        for (const auto& sequence : run_data.operation_parameters) {
          if (current_table_index == sequence.at(table_offset) &&
              current_offset == sequence.at(offset_offset)) {
            current_sequences.push_back(sequence.at(count_offset));
            current_offset += sequence.at(count_offset);
          } else {
            physical_addresses_map.insert(
                {table_memory_blocks
                         .at(run_data.input_data_definition_files.at(
                             current_table_index))
                         ->GetPhysicalAddress() +
                     current_offset,
                 std::move(current_sequences)});
            current_sequences = {sequence.at(count_offset)};
            current_table_index = sequence.at(table_offset);
            current_offset =
                sequence.at(offset_offset) + sequence.at(count_offset);
          }
        }
        physical_addresses_map.insert(
            {table_memory_blocks
                     .at(run_data.input_data_definition_files.at(
                         current_table_index))
                     ->GetPhysicalAddress() +
                 current_offset,
             std::move(current_sequences)});
        // virtual channel count is the size of the vector - We assume this is
        // correct with the overall parameters.
        virtual_channel_count = run_data.operation_parameters.size();
      } else {
        auto physical_address_ptr =
            table_memory_blocks.at(table_name)->GetPhysicalAddress();
        physical_addresses_map.insert({physical_address_ptr, {-1}});
      }
    } else if (!is_input &&
               !run_data.output_data_definition_files.at(stream_index)
                    .empty()) {
      auto physical_address_ptr =
          table_memory_blocks.at(table_name)->GetPhysicalAddress() +
          run_data.output_offset.at(stream_index);
      physical_addresses_map.insert({physical_address_ptr, {-1}});
    }

    int chunk_count = -1;
    auto chunk_count_def =
        node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                           kIOStreamParamDefs.kChunkCountOffset);
    if (!chunk_count_def.empty()) {
      chunk_count = chunk_count_def.at(0);
    }

    // The stream sizes come from the main + table data.
    StreamDataParameters current_stream_parameters = {
        stream_ids[stream_index],
        current_tables_metadata.at(table_name).record_size,
        current_tables_metadata.at(table_name).record_count,
        physical_addresses_map,
        node_parameters.at(stream_index * kIOStreamParamDefs.kStreamParamCount +
                           kIOStreamParamDefs.kProjectionOffset),
        chunk_count,
        virtual_channel_count};

    parameters_for_acceleration.push_back(current_stream_parameters);
  }
  return parameters_for_acceleration;
}

void QueryManager::StoreStreamResultParameters(
    std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::vector<int>& stream_ids, const QueryNode* node,
    const NodeRunData& run_data) {
  std::vector<StreamResultParameters> result_parameters_vector;
  for (int stream_index = 0; stream_index < stream_ids.size(); stream_index++) {
    if (!run_data.output_data_definition_files.at(stream_index).empty()) {
      result_parameters_vector.emplace_back(
          stream_index, stream_ids[stream_index],
          run_data.output_data_definition_files.at(stream_index),
          node->is_checked.at(stream_index),
          node->next_nodes.at(stream_index) != nullptr,
          node->given_operation_parameters.output_stream_parameters,
          run_data.output_offset.at(stream_index));
    }
  }
  result_parameters.insert({node->node_name, result_parameters_vector});
}

auto QueryManager::SetupAccelerationNodesForExecution(
    DataManagerInterface* data_manager, MemoryManagerInterface* memory_manager,
    AcceleratorLibraryInterface* accelerator_library,
    const std::vector<QueryNode*>& current_query_nodes,
    const std::map<std::string, TableMetadata>& current_tables_metadata,
    std::unordered_map<std::string, MemoryBlockInterface*>& table_memory_blocks,
    std::unordered_map<std::string, int>& table_counter)
    -> std::pair<std::vector<AcceleratedQueryNode>,
                 std::map<std::string, std::vector<StreamResultParameters>>> {
  std::map<std::string, std::vector<StreamResultParameters>> result_parameters;
  std::vector<AcceleratedQueryNode> query_nodes;

  std::map<std::string, std::vector<int>> output_ids;
  std::map<std::string, std::vector<int>> input_ids;

  IDManager::AllocateStreamIDs(current_query_nodes, input_ids, output_ids);

  for (auto& node : current_query_nodes) {
    auto current_run_params = std::move(node->module_run_data.back());
    node->module_run_data.pop_back();
    for (const auto& table : current_run_params.input_data_definition_files) {
      if (!table.empty()) {
        table_counter[table]--;
      }
    }
    for (const auto& table : current_run_params.output_data_definition_files) {
      if (!table.empty()) {
        table_counter[table]--;
      }
    }
    AllocateOutputMemoryBlocks(memory_manager, current_run_params,
                               table_memory_blocks);
    AllocateInputMemoryBlocks(
        memory_manager, data_manager, current_run_params,
        node->given_operation_parameters.input_stream_parameters,
        current_tables_metadata, table_memory_blocks);

    auto input_params = CreateStreamParams(
        true, node, input_ids[node->node_name], current_run_params,
        current_tables_metadata, table_memory_blocks);
    auto output_params = CreateStreamParams(
        false, node, output_ids[node->node_name], current_run_params,
        current_tables_metadata, table_memory_blocks);

    AddQueryNodes(query_nodes, std::move(input_params),
                  std::move(output_params), node, current_run_params);

    StoreStreamResultParameters(result_parameters, output_ids[node->node_name],
                                node, current_run_params);
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
    const std::unordered_set<std::string>& first_node_names,
    const std::unordered_set<std::string>& starting_nodes,
    std::unordered_set<std::string>& processed_nodes,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& tables,
    AcceleratorLibraryInterface& drivers, const Config& config,
    NodeSchedulerInterface& node_scheduler,
    std::vector<ScheduledModule>& current_configuration) {
  node_scheduler.BenchmarkScheduling(
      first_node_names, starting_nodes, processed_nodes, graph, drivers, tables,
      current_configuration, config, benchmark_stats_);
}

void QueryManager::PrintBenchmarkStats() {
  /*for (const auto& [stats_key,value] : benchmark_stats_) {
    std::cout << stats_key << ": " << value << std::endl;
  }*/
  json_reader_->WriteValueMap(benchmark_stats_, "benchmark_stats.json");
}

auto QueryManager::ScheduleNextSetOfNodes(
    std::vector<QueryNode*>& query_nodes,
    const std::unordered_set<std::string>& first_node_names,
    const std::unordered_set<std::string>& starting_nodes,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& tables,
    AcceleratorLibraryInterface& drivers, const Config& config,
    NodeSchedulerInterface& node_scheduler,
    const std::vector<ScheduledModule>& current_configuration,
    std::unordered_set<std::string>& skipped_nodes,
    std::unordered_map<std::string, int>& table_counter)
    -> std::queue<
        std::pair<std::vector<ScheduledModule>, std::vector<QueryNode*>>> {
  return node_scheduler.GetNextSetOfRuns(
      query_nodes, first_node_names, starting_nodes, graph, drivers, tables,
      current_configuration, config, skipped_nodes, table_counter);
}

auto QueryManager::GetPRBitstreamsToLoadWithPassthroughModules(
    std::vector<ScheduledModule>& current_config,
    const std::vector<ScheduledModule>& next_config, int column_count)
    -> std::pair<std::vector<std::string>,
                 std::vector<std::pair<QueryOperationType, bool>>> {
  std::vector<int> written_frames(column_count, 0);

  auto old_routing_modules = BitstreamConfigHelper::GetOldNonOverlappingModules(
      next_config, current_config);

  auto [reduced_next_config, reduced_current_config] =
      BitstreamConfigHelper::GetConfigCompliments(next_config, current_config);

  auto removable_modules = reduced_current_config;
  for (const auto& reused_module : old_routing_modules) {
    for (const auto& removable_module : reduced_current_config) {
      removable_modules.erase(
          std::remove(removable_modules.begin(), removable_modules.end(),
                      reused_module),
          removable_modules.end());
    }
  }

  // Find out which frames need writing to.
  for (const auto& module : removable_modules) {
    for (int column_i = module.position.first;
         column_i < module.position.second + 1; column_i++) {
      written_frames[column_i] = 1;
    }
  }
  // for (const auto& module : old_routing_modules) {
  //  for (int column_i = module.position.first;
  //       column_i < module.position.second + 1; column_i++) {
  //    written_frames[column_i] = false;
  //  }
  //}

  std::vector<std::string> required_bitstreams;
  for (const auto& module : reduced_next_config) {
    for (int column_i = module.position.first;
         column_i < module.position.second + 1; column_i++) {
      written_frames[column_i] = 0;
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
  /*std::vector<std::string> routing_bitstreams = {
      "RT_95.bin", "RT_92.bin", "RT_89.bin", "RT_86.bin", "RT_83.bin",
      "RT_80.bin", "RT_77.bin", "RT_74.bin", "RT_71.bin", "RT_68.bin",
      "RT_65.bin", "RT_62.bin", "RT_59.bin", "RT_56.bin", "TAA_53.bin"};*/

  for (int column_i = 0; column_i < routing_bitstreams.size(); column_i++) {
    if (written_frames.at(column_i)) {
      /*required_bitstreams.push_back("TAA_53.bin");
      break;*/
      required_bitstreams.push_back(routing_bitstreams.at(column_i));
    }
  }

  auto left_over_config = BitstreamConfigHelper::GetResultingConfig(
      current_config, removable_modules, reduced_next_config);
  std::sort(left_over_config.begin(), left_over_config.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs.position.first < rhs.position.first;
            });

  std::vector<std::pair<QueryOperationType, bool>> passthrough_modules;
  for (const auto& module : left_over_config) {
    if (std::find_if(next_config.begin(), next_config.end(),
                     [&](auto new_module) {
                       return module.bitstream == new_module.bitstream;
                     }) != next_config.end()) {
      passthrough_modules.emplace_back(module.operation_type, false);
    } else {
      passthrough_modules.emplace_back(module.operation_type, true);
    }
  }

  current_config = left_over_config;

  /*for (const auto& module : old_routing_modules) {
    required_bitstreams.push_back(module.bitstream);
  }*/

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
    MemoryBlockInterface* memory_device, int row_count,
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
    MemoryBlockInterface* memory_device, int row_count,
    const std::string& filename,
    const std::vector<std::vector<int>>& node_parameters, int stream_index) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  auto resulting_table = TableManager::ReadTableFromMemory(
      data_manager, node_parameters, stream_index, memory_device, row_count);
  TableManager::WriteResultTableFile(data_manager, resulting_table, filename);

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "MEMORY TO FS WRITE:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
  Log(LogLevel::kInfo,
      "Write result data time = " +
          std::to_string(
              std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                  .count()) +
          "[ms]");
}

void QueryManager::CopyMemoryData(int table_size,
                                  MemoryBlockInterface* source_memory_device,
                                  MemoryBlockInterface* target_memory_device) {
  volatile uint32_t* source = source_memory_device->GetVirtualAddress();
  volatile uint32_t* target = target_memory_device->GetVirtualAddress();
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  for (int i = 0; i < table_size; i++) {
    target[i] = source[i];
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "MEMORY TO MEMORY WRITE:"
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     begin)
                   .count()
            << std::endl;
}

void QueryManager::ProcessResults(
    const DataManagerInterface* data_manager,
    const std::array<int, query_acceleration_constants::kMaxIOStreamCount>
        result_sizes,
    const std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::unordered_map<std::string, MemoryBlockInterface*>&
        table_memory_blocks,
    std::map<std::string, TableMetadata>& scheduling_table_data) {
  for (auto const& [node_name, result_parameter_vector] : result_parameters) {
    for (auto const& result_params : result_parameter_vector) {
      // Assuming there's nothing after the offset and everything is valid
      // before offset!
      int record_count = result_sizes.at(result_params.output_id) +
                         result_params.output_offset;
      Log(LogLevel::kDebug,
          node_name + "_" + std::to_string(result_params.stream_index) +
              " has " +
              std::to_string(record_count - result_params.output_offset) +
              " rows!");
      scheduling_table_data[result_params.filename].record_count = record_count;
      if (!result_params.update_table_sizes_only) {
        if (result_params.check_results) {
          CheckResults(
              data_manager, table_memory_blocks.at(result_params.filename),
              record_count, result_params.filename,
              result_params.stream_specifications, result_params.stream_index);
        } else {
          WriteResults(
              data_manager, table_memory_blocks.at(result_params.filename),
              record_count, result_params.filename,
              result_params.stream_specifications, result_params.stream_index);
        }
      }
    }
  }
}

void QueryManager::FreeMemoryBlocks(
    MemoryManagerInterface* memory_manager,
    std::map<std::string, std::vector<MemoryBlockInterface*>>&
        input_memory_blocks,
    std::map<std::string, std::vector<MemoryBlockInterface*>>&
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>& input_stream_sizes,
    std::map<std::string, std::vector<RecordSizeAndCount>>& output_stream_sizes,
    const std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links,
    const std::vector<std::string>& scheduled_node_names) {
  // For all reuse links where it is doing self linking. Just write output to
  // input and don't clear the memory block. Then remove the link from the
  // vector.
  // vec.erase(std::remove(vec.begin(), vec.end(), 8), vec.end());

  std::vector<std::string> save_input;
  for (const auto& [node_name, data_mapping] : reuse_links) {
    for (const auto& [output_stream_index, target_input_streams] :
         data_mapping) {
      if (target_input_streams.size() == 1) {
        auto target_node_name = target_input_streams.at(0).first;
        auto target_stream_index = target_input_streams.at(0).second;
        if (target_node_name == node_name) {
          if (output_memory_blocks[node_name][output_stream_index] !=
              input_memory_blocks[target_node_name][target_stream_index]) {
            CopyMemoryData(
                output_stream_sizes[node_name][output_stream_index].first *
                    output_stream_sizes[node_name][output_stream_index].second,
                output_memory_blocks[node_name][output_stream_index],
                input_memory_blocks[target_node_name][target_stream_index]);
          } else {
            output_memory_blocks[node_name][output_stream_index] = nullptr;
          }
          save_input.push_back(target_node_name);
        }
      }
    }
  }

  // Check when does stuff get saved
  /*std::vector<std::string> removable_vectors;*/
  for (const auto& name : scheduled_node_names) {
    if (std::find(save_input.begin(), save_input.end(), name) ==
        save_input.end()) {
      for (auto& memory_block : input_memory_blocks[name]) {
        if (memory_block) {
          memory_manager->FreeMemoryBlock(std::move(memory_block));
        }
        memory_block = nullptr;
      }
      // Should only be erased when it's not a target in reuse_links
      // input_stream_sizes.erase(name);
      // input_memory_blocks.erase(name);
    }
  }

  for (const auto& [node_name, data_mapping] : reuse_links) {
    if (std::find(save_input.begin(), save_input.end(), node_name) ==
        save_input.end()) {
      for (const auto& [output_stream_index, target_input_streams] :
           data_mapping) {
        if (target_input_streams.size() == 1) {
          auto target_node_name = target_input_streams.at(0).first;
          auto target_stream_index = target_input_streams.at(0).second;
          /*removable_vectors.erase(
              std::remove(removable_vectors.begin(), removable_vectors.end(),
                          target_node_name),
              removable_vectors.end());*/
          input_memory_blocks[target_node_name][target_stream_index] =
              std::move(output_memory_blocks[node_name][output_stream_index]);
          output_memory_blocks[node_name][output_stream_index] = nullptr;
          input_stream_sizes[target_node_name][target_stream_index] =
              output_stream_sizes[node_name][output_stream_index];
        } else {
          for (const auto& [target_node_name, target_stream_index] :
               target_input_streams) {
            /*removable_vectors.erase(
                std::remove(removable_vectors.begin(), removable_vectors.end(),
                            target_node_name),
                removable_vectors.end());*/
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
    MemoryManagerInterface* memory_manager, FPGAManagerInterface* fpga_manager,
    const DataManagerInterface* data_manager,
    std::unordered_map<std::string, MemoryBlockInterface*>& table_memory_blocks,
    const std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::vector<AcceleratedQueryNode>& execution_query_nodes,
    std::map<std::string, TableMetadata>& scheduling_table_data,
    std::unordered_map<std::string, int>& table_counter) {
  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  Log(LogLevel::kTrace, "Setup query!");
  fpga_manager->SetupQueryAcceleration(execution_query_nodes);
  std::chrono::steady_clock::time_point init_end =
      std::chrono::steady_clock::now();
  std::cout << "INITIALISATION:"
            << std::chrono::duration_cast<std::chrono::microseconds>(init_end -
                                                                     begin)
                   .count()
            << std::endl;
  Log(LogLevel::kTrace, "Running query!");
  auto result_sizes = fpga_manager->RunQueryAcceleration();
  Log(LogLevel::kTrace, "Query done!");

  std::chrono::steady_clock::time_point total_end =
      std::chrono::steady_clock::now();
  std::cout << "TOTAL EXEC:"
            << std::chrono::duration_cast<std::chrono::microseconds>(total_end -
                                                                     begin)
                   .count()
            << std::endl;
  Log(LogLevel::kInfo,
      "Init and run time = " +
          std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
                             total_end - begin)
                             .count()) +
          "[microseconds]");

  ProcessResults(data_manager, result_sizes, result_parameters,
                 table_memory_blocks, scheduling_table_data);
  std::vector<std::string> removable_tables;
  for (const auto& [table_name, counter] : table_counter) {
    if (counter == 0) {
      removable_tables.push_back(table_name);
    }
  }
  for (const auto& table_name : removable_tables) {
    table_counter.erase(table_name);
    scheduling_table_data.erase(table_name);
    memory_manager->FreeMemoryBlock(table_memory_blocks.at(table_name));
    table_memory_blocks.erase(table_name);
  }
}

void QueryManager::UpdateTableData(
    const std::map<std::string, std::vector<StreamResultParameters>>&
        result_parameters,
    const std::map<std::string, std::vector<RecordSizeAndCount>>&
        output_stream_sizes,
    std::map<std::string, TableMetadata>& scheduling_table_data,
    const std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links,
    std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph) {
  // Only thing we need to do is crop sorted status
  // And set record counts.

  // TODO(Kaspar): This needs to get tested!!!
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
    const std::string& filename) {
  auto& current_data = scheduling_table_data.at(filename);
  if (!current_data.sorted_status.empty()) {
    if (current_data.sorted_status.size() == 1) {
      current_data.sorted_status[0] = current_data.record_count;
    } else {
      if ((current_data.sorted_status.at(1) - 1) *
                  current_data.sorted_status.at(2) +
              current_data.sorted_status.at(0) >
          current_data.record_count) {
        if (current_data.record_count < current_data.sorted_status.at(0)) {
          current_data.sorted_status.clear();
          current_data.sorted_status.push_back(current_data.record_count);
        } else {
          int left_over_rows_to_sort =
              current_data.record_count - current_data.sorted_status.at(0);
          int left_over_sequence_count =
              left_over_rows_to_sort / current_data.sorted_status.at(2);
          if (left_over_sequence_count == 0) {
            current_data.sorted_status[1] = 1;
            current_data.sorted_status[2] = left_over_rows_to_sort;
          } else {
            if (left_over_sequence_count * current_data.sorted_status.at(2) !=
                left_over_rows_to_sort) {
              left_over_sequence_count++;
            }
            current_data.sorted_status[1] = left_over_sequence_count;
          }
        }
      }  // Else it's all fine!
    }
  }
}

void QueryManager::AddQueryNodes(
    std::vector<AcceleratedQueryNode>& query_nodes_vector,
    std::vector<StreamDataParameters>&& input_params,
    std::vector<StreamDataParameters>&& output_params, QueryNode* node,
    const NodeRunData& run_data) {
  std::vector<std::vector<int>> operation_params;
  if (node->operation_type == QueryOperationType::kMergeSort) {
    operation_params.push_back(
        {static_cast<int>(run_data.module_locations.size()), 1});
    for (int i = 0; i < run_data.module_locations.size(); i++) {
      operation_params.push_back(
          node->given_operation_parameters.operation_parameters.at(
              run_data.run_index));
    }
    // Just for debugging marking used operations params.
    node->given_operation_parameters.operation_parameters[run_data.run_index]
        .push_back(-1);
  } else {
    operation_params = node->given_operation_parameters.operation_parameters;
  }
  // Assume the locations are sorted.
  const auto& sorted_module_locations = run_data.module_locations;
  auto no_io_input_params = input_params;
  for (auto& stream_param : no_io_input_params) {
    stream_param.physical_addresses_map.clear();
  }
  auto no_io_output_params = output_params;
  for (auto& stream_param : no_io_output_params) {
    stream_param.physical_addresses_map.clear();
  }
  if (sorted_module_locations.size() == 1) {
    query_nodes_vector.push_back({std::move(input_params),
                                  std::move(output_params),
                                  node->operation_type,
                                  sorted_module_locations.at(0),
                                  {},
                                  operation_params});
  } else if (operation_params.empty()) {
    // Multiple composed modules
    for (int i = 0; i < sorted_module_locations.size(); i++) {
      auto current_input = (i == 0) ? input_params : no_io_input_params;
      auto current_output = (i == sorted_module_locations.size() - 1)
                                ? output_params
                                : no_io_output_params;
      query_nodes_vector.push_back({current_input, current_output,
                                    node->operation_type,
                                    sorted_module_locations.at(i),
                                    sorted_module_locations, operation_params});
    }
  } else {
    // Multiple resource elastic modules.
    // Hardcoded for merge_sort for now.
    /*for (int i = 0; i < sorted_module_locations.size(); i++) {
      auto current_input = (i == 0) ? input_params : no_io_input_params;
      auto current_output = (i == sorted_module_locations.size() - 1)
                                ? output_params
                                : no_io_output_params;
      query_nodes_vector.push_back(
          {current_input, current_output, node.operation_type,
           sorted_module_locations.at(i), sorted_module_locations,
           {{node.operation_parameters.operation_parameters[0][0] *
                 static_cast<int>(sorted_module_locations.size()),
             node.operation_parameters.operation_parameters[0][1]}}});
    }*/

    auto all_module_params = operation_params;
    if (all_module_params.at(0).at(0) != sorted_module_locations.size()) {
      throw std::runtime_error("Wrong parameters given!");
    }
    int offset = all_module_params.at(0).at(1);
    for (int i = 0; i < sorted_module_locations.size(); i++) {
      auto current_input = (i == 0) ? input_params : no_io_input_params;
      auto current_output = (i == sorted_module_locations.size() - 1)
                                ? output_params
                                : no_io_output_params;
      std::vector<std::vector<int>> single_module_params = {
          all_module_params.begin() + 1 + offset * i,
          all_module_params.begin() + 1 + offset * (i + 1)};
      single_module_params.insert(single_module_params.begin(),
                                  all_module_params[0]);
      query_nodes_vector.push_back(
          {current_input, current_output, node->operation_type,
           sorted_module_locations.at(i), sorted_module_locations,
           single_module_params});
    }
  }
}
