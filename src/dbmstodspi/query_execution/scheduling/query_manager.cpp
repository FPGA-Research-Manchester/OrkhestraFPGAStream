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
#include <set>
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
using orkhestrafs::dbmstodspi::StreamDataParameters;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::dbmstodspi::query_acceleration_constants::kIOStreamParamDefs;

auto QueryManager::GetData() -> std::vector<long> {
  return {data_count_, static_configuration_, initialisation_sum_,
          scheduling_sum_};
}
auto QueryManager::GetConfigTime() -> long { return latest_config_; }

void QueryManager::MeasureBitstreamConfigurationSpeed(
    const std::map<QueryOperationType, OperationPRModules>& hw_library,
    MemoryManagerInterface* memory_manager) {
  std::set<std::string> bitstreams_to_measure;
  for (const auto& [operation, bitstreams_info] : hw_library) {
    for (const auto& [bitstream_name, bitstream_info] :
         bitstreams_info.bitstream_map) {
      bitstreams_to_measure.insert(bitstream_name);
    }
  }
  // TODO(Kaspar): Move this elsewhere
  std::set<std::string> routing_bitstreams = {
      "RT_95.bin",  "RT_92.bin",  "RT_89.bin",  "RT_86.bin",  "RT_83.bin",
      "RT_80.bin",  "RT_77.bin",  "RT_74.bin",  "RT_71.bin",  "RT_68.bin",
      "RT_65.bin",  "RT_62.bin",  "RT_59.bin",  "RT_56.bin",  "RT_53.bin",
      "RT_50.bin",  "RT_47.bin",  "RT_44.bin",  "RT_41.bin",  "RT_38.bin",
      "RT_35.bin",  "RT_32.bin",  "RT_29.bin",  "RT_26.bin",  "RT_23.bin",
      "RT_20.bin",  "RT_17.bin",  "RT_14.bin",  "RT_11.bin",  "RT_8.bin",
      "RT_5.bin",   "RT_2.bin",   "TAA_95.bin", "TAA_92.bin", "TAA_89.bin",
      "TAA_86.bin", "TAA_83.bin", "TAA_80.bin", "TAA_77.bin", "TAA_74.bin",
      "TAA_71.bin", "TAA_68.bin", "TAA_65.bin", "TAA_62.bin", "TAA_59.bin",
      "TAA_56.bin", "TAA_53.bin", "TAA_50.bin", "TAA_47.bin", "TAA_44.bin",
      "TAA_41.bin", "TAA_38.bin", "TAA_35.bin", "TAA_32.bin", "TAA_29.bin",
      "TAA_26.bin", "TAA_23.bin", "TAA_20.bin", "TAA_17.bin", "TAA_14.bin",
      "TAA_11.bin", "TAA_8.bin",  "TAA_5.bin",  "TAA_2.bin"};
  bitstreams_to_measure.merge(routing_bitstreams);
  // bitstreams_to_measure.insert("byteman_PRregionRTandTA_0_96.bin");
  // bitstreams_to_measure.insert("byteman_MergeSort128_bitstreamSizeTest_7_42.bin");
  // bitstreams_to_measure.insert("byteman_MergeSort128_bitstreamSizeTest_37_72.bin");
  memory_manager->MeasureConfigurationSpeed(bitstreams_to_measure);
}

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
          memory_manager->GetAvailableMemoryBlock();
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
          memory_manager->GetAvailableMemoryBlock();
      std::chrono::steady_clock::time_point begin =
          std::chrono::steady_clock::now();

      auto [record_size, record_count] = TableManager::WriteDataToMemory(
          data_manager, input_stream_parameters, stream_index,
          table_memory_blocks[input_table], input_table);

      std::chrono::steady_clock::time_point end =
          std::chrono::steady_clock::now();
      std::cout << "FS TO MEMORY WRITE: "
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
    std::string table_name;
    if (is_input) {
      table_name = run_data.input_data_definition_files.at(stream_index);
    } else {
      table_name = run_data.output_data_definition_files.at(stream_index);
    }
    std::map<volatile uint32_t*, std::vector<int>> physical_addresses_map;
    int virtual_channel_count = -1;
    if (is_input && !table_name.empty()) {
      if (node->operation_type == QueryOperationType::kMergeSort) {
        // For debugging
        int current_record_count = 0;

        int current_record_size =
            current_tables_metadata.at(table_name).record_size;
        std::vector<int> current_sequences;
        int current_table_index =
            run_data.operation_parameters.front().at(table_offset);
        int current_offset =
            run_data.operation_parameters.front().at(offset_offset);
        int write_offset = current_offset;
        for (const auto& sequence : run_data.operation_parameters) {
          if (current_table_index == sequence.at(table_offset) &&
              current_offset == sequence.at(offset_offset)) {
            current_sequences.push_back(sequence.at(count_offset));
            current_record_count += sequence.at(count_offset);
            current_offset += sequence.at(count_offset);
          } else {
            physical_addresses_map.insert(
                {table_memory_blocks
                         .at(run_data.input_data_definition_files.at(
                             current_table_index))
                         ->GetPhysicalAddress() +
                     write_offset * current_record_size,
                 std::move(current_sequences)});

            current_sequences = {sequence.at(count_offset)};
            current_table_index = sequence.at(table_offset);
            write_offset = sequence.at(offset_offset);
            current_offset =
                sequence.at(offset_offset) + sequence.at(count_offset);
            current_record_count += sequence.at(count_offset);
          }
        }
        // virtual channel count is the size of the vector - We assume this is
        // correct with the overall parameters.
        virtual_channel_count = run_data.operation_parameters.size();
        // Need to make virtual channel count be equal to the sum of the
        // modules. Need to add the missing amount of 0 reads.
        int max_channel_count = 0;
        for (const auto& capacity :
             node->given_operation_parameters.operation_parameters.at(
                 run_data.run_index)) {
          max_channel_count += capacity;
        }
        std::vector<int> empty_sequences(
            max_channel_count - virtual_channel_count, 0);
        current_sequences.insert(current_sequences.end(),
                                 empty_sequences.begin(),
                                 empty_sequences.end());

        physical_addresses_map.insert(
            {table_memory_blocks
                     .at(run_data.input_data_definition_files.at(
                         current_table_index))
                     ->GetPhysicalAddress() +
                 write_offset * current_record_size,
             std::move(current_sequences)});

        virtual_channel_count = max_channel_count;

        merge_count_ += current_record_count;
        /*std::cout << "Streamed data (rows): "
                  << std::to_string(current_record_count)
                  << std::endl;
        std::cout
            << "Streamed data (bytes): "
            << std::to_string(
                   static_cast<long>(
                       current_tables_metadata.at(table_name).record_size) *
                   static_cast<long>(merge_count) *
                   static_cast<long>(4))
            << std::endl;*/
        data_count_ += static_cast<long>(
                          current_tables_metadata.at(table_name).record_size) *
                      static_cast<long>(merge_count_) * static_cast<long>(4);

      } else {
        /*std::cout << "Streamed data (rows): "
                  << std::to_string(
                         current_tables_metadata.at(table_name).record_count)
                  << std::endl;
        std::cout
            << "Streamed data (bytes): "
            << std::to_string(
                   static_cast<long>(
                       current_tables_metadata.at(table_name).record_size) *
                   static_cast<long>(
                       current_tables_metadata.at(table_name).record_count) *
                   static_cast<long>(4))
            << std::endl;*/
        data_count_ += static_cast<long>(
                          current_tables_metadata.at(table_name).record_size) *
                      static_cast<long>(
                          current_tables_metadata.at(table_name).record_count) *
                      static_cast<long>(4);
        //if (is_last_) {
        //  std::cout << "Streamed data (bytes): " << std::to_string(data_count)
        //            << std::endl;
        //}
        
        auto* physical_address_ptr =
            table_memory_blocks.at(table_name)->GetPhysicalAddress();
        physical_addresses_map.insert({physical_address_ptr, {-1}});
      }
    } else if (!is_input && !table_name.empty()) {
      auto* physical_address_ptr =
          table_memory_blocks.at(table_name)->GetPhysicalAddress() +
          run_data.output_offset.at(stream_index) *
              current_tables_metadata.at(table_name).record_size;
      physical_addresses_map.insert({physical_address_ptr, {-1}});
    } else {
      // Leave address map empty!
      if (is_input) {
        table_name = node->given_input_data_definition_files.at(stream_index);
      } else {
        table_name = node->given_output_data_definition_files.at(stream_index);
      }
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
          node->next_nodes.at(stream_index) != nullptr &&
              !node->is_checked.at(stream_index),
          node->given_operation_parameters.output_stream_parameters,
          run_data.output_offset.at(stream_index));
    }
  }
  result_parameters.insert({node->node_name, result_parameters_vector});
}

auto QueryManager::SetupAccelerationNodesForExecution(
    DataManagerInterface* data_manager, MemoryManagerInterface* memory_manager,
    AcceleratorLibraryInterface* /*accelerator_library*/,
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

  for (const auto& node : current_query_nodes) {
    auto current_run_params = std::move(node->module_run_data.front());
    node->module_run_data.pop_front();
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

  // TODO: Look into removing this sort!
  std::sort(query_nodes.begin(), query_nodes.end(),
            [](AcceleratedQueryNode const& lhs,
               AcceleratedQueryNode const& rhs) -> bool {
              return lhs.operation_module_location <
                     rhs.operation_module_location;
            });

  return {std::move(query_nodes), std::move(result_parameters)};
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
  static_configuration_ = memory_manager->GetTime();
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

auto QueryManager::LoadPRBitstreams(
    MemoryManagerInterface* memory_manager,
    const std::vector<std::string>& bitstream_names,
    AcceleratorLibraryInterface& driver_library) ->long {
  if (!bitstream_names.empty()) {
    auto dma_module = driver_library.GetDMAModule();
    memory_manager->LoadPartialBitstream(bitstream_names, *dma_module);
    return memory_manager->GetTime();
  } else {
    return 0;
  }
}

void QueryManager::BenchmarkScheduling(
    const std::unordered_set<std::string>& first_node_names,
    const std::unordered_set<std::string>& starting_nodes,
    std::unordered_set<std::string>& processed_nodes,
    const std::unordered_map<std::string, SchedulingQueryNode>& graph,
    std::map<std::string, TableMetadata>& tables,
    AcceleratorLibraryInterface& drivers, const Config& config,
    NodeSchedulerInterface& node_scheduler,
    std::vector<ScheduledModule>& current_configuration,
    const std::unordered_set<std::string>& blocked_nodes) {
  node_scheduler.BenchmarkScheduling(
      first_node_names, starting_nodes, processed_nodes, graph, drivers, tables,
      current_configuration, config, benchmark_stats_, blocked_nodes);
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
    std::unordered_map<std::string, int>& table_counter,
    const std::unordered_set<std::string>& blocked_nodes)
    -> std::queue<
        std::pair<std::vector<ScheduledModule>, std::vector<QueryNode*>>> {
  auto thing = node_scheduler.GetNextSetOfRuns(
      query_nodes, first_node_names, starting_nodes, graph, drivers, tables,
      current_configuration, config, skipped_nodes, table_counter,
      blocked_nodes);
  scheduling_sum_ += node_scheduler.GetTime();
  return thing;
}

auto QueryManager::GetPRBitstreamsToLoadWithPassthroughModules(
    std::vector<ScheduledModule>& current_config,
    const std::vector<ScheduledModule>& next_config,
    std::vector<std::string>& current_routing)
    -> std::pair<std::vector<std::string>,
                 std::vector<std::pair<QueryOperationType, bool>>> {
  std::vector<int> written_frames(routing_bitstreams_.size(), 0);

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
      current_routing[column_i] = "";
    }
  }

  std::vector<std::string> required_bitstreams;
  for (const auto& new_module : reduced_next_config) {
    for (int column_i = new_module.position.first;
         column_i < new_module.position.second + 1; column_i++) {
      written_frames[column_i] = 0;
      current_routing[column_i] = new_module.bitstream;
    }
    required_bitstreams.push_back(new_module.bitstream);
  }

  auto left_over_config = BitstreamConfigHelper::GetResultingConfig(
      current_config, removable_modules, reduced_next_config);
  std::sort(left_over_config.begin(), left_over_config.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs.position.first < rhs.position.first;
            });

  std::vector<std::pair<QueryOperationType, bool>> passthrough_modules;

  // 1.Find out how far does it have to go
  int furthest_required_column = next_config.back().position.second;
  // 2.Check if there is a connection from beginning - if not add RT
  // And find all passthrough modules
  std::unordered_map<std::string, QueryOperationType> old_operations;
  for (const auto& prev_module : current_config) {
    old_operations.insert({prev_module.bitstream, prev_module.operation_type});
  }
  std::unordered_map<std::string, QueryOperationType> new_operations;
  for (const auto& new_module : next_config) {
    new_operations.insert({new_module.bitstream, new_module.operation_type});
  }

  std::string last_seen_bitstream = "";
  for (int column_i = 0; column_i <= furthest_required_column; column_i++) {
    if (current_routing[column_i].empty() ||
        current_routing[column_i] == "TAA") {
      current_routing[column_i] = "RT";
      required_bitstreams.push_back(routing_bitstreams_.at(column_i));
    } else if (last_seen_bitstream == current_routing[column_i] ||
               current_routing[column_i] == "RT") {
      // Do nothing
    } else if (last_seen_bitstream != current_routing[column_i] &&
               new_operations.find(current_routing[column_i]) !=
                   new_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
      passthrough_modules.emplace_back(
          new_operations.at(current_routing[column_i]), false);
    } else if (last_seen_bitstream != current_routing[column_i] &&
               old_operations.find(current_routing[column_i]) !=
                   old_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
      passthrough_modules.emplace_back(
          old_operations.at(current_routing[column_i]), true);
    } else {
      throw std::runtime_error("Unknown routing bitstream");
    }
  }
  // 3.Check if there is a connection to TAA or end from the end - if not add
  // TAA
  last_seen_bitstream = "";
  for (int column_i = furthest_required_column + 1;
       column_i < routing_bitstreams_.size(); column_i++) {
    if (current_routing[column_i].empty()) {
      current_routing[column_i] = "TAA";
      required_bitstreams.push_back(turnaround_bitstreams_.at(column_i));
      break;
    } else if (current_routing[column_i] == "TAA") {
      break;
    } else if (last_seen_bitstream == current_routing[column_i] ||
               current_routing[column_i] == "RT") {
      // Do nothing
    } else if (last_seen_bitstream != current_routing[column_i] &&
               new_operations.find(current_routing[column_i]) !=
                   new_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
      passthrough_modules.emplace_back(
          new_operations.at(current_routing[column_i]), false);
    } else if (last_seen_bitstream != current_routing[column_i] &&
               old_operations.find(current_routing[column_i]) !=
                   old_operations.end()) {
      last_seen_bitstream = current_routing[column_i];
      passthrough_modules.emplace_back(
          old_operations.at(current_routing[column_i]), true);
    } else {
      throw std::runtime_error("Unknown routing bitstream");
    }
  }

  /*for (int column_i = furthest_required_column + 1;
       column_i < routing_bitstreams_.size(); column_i++) {
    current_routing[column_i] = "RT";
    required_bitstreams.push_back(routing_bitstreams_.at(column_i));
  }
  required_bitstreams.push_back(routing_bitstreams_.at(0));*/
  // Implementation without using TAAs
  /*GetRoutingBitstreamsAndPassthroughBitstreams(
      written_frames, required_bitstreams, left_over_config, next_config,
      passthrough_modules);*/

  current_config = left_over_config;

  return {required_bitstreams, passthrough_modules};
}

void QueryManager::GetRoutingBitstreamsAndPassthroughBitstreams(
    const std::vector<int>& written_frames,
    std::vector<std::string>& required_bitstreams,
    const std::vector<ScheduledModule>& left_over_config,
    const std::vector<ScheduledModule>& next_config,
    std::vector<std::pair<QueryOperationType, bool>>& passthrough_modules) {
  // Get the bitstreams
  for (int column_i = 0; column_i < routing_bitstreams_.size(); column_i++) {
    if (written_frames.at(column_i)) {
      required_bitstreams.push_back(routing_bitstreams_.at(column_i));
    }
  }
  // Get Passthrough modules
  for (const auto& new_current_module : left_over_config) {
    if (std::find_if(
            next_config.begin(), next_config.end(), [&](auto new_module) {
              return new_current_module.bitstream == new_module.bitstream;
            }) != next_config.end()) {
      passthrough_modules.emplace_back(new_current_module.operation_type,
                                       false);
    } else {
      passthrough_modules.emplace_back(new_current_module.operation_type, true);
    }
  }
}

void QueryManager::CheckTableData(const DataManagerInterface* data_manager,
                                  const TableData& expected_table,
                                  const TableData& resulting_table) {
  if (expected_table == resulting_table) {
    Log(LogLevel::kDebug, "Query results are correct!");
  } else {
    // TODO: Fix it throwing error with null elements in the table!
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
    // data_manager->PrintTableData(resulting_table);
    // throw std::runtime_error("Failed table check!");
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
              " new resulting rows!");
      scheduling_table_data[result_params.filename].record_count = record_count;
      if (!result_params.update_table_sizes_only) {
        if (result_params.check_results) {
          CheckResults(
              data_manager, table_memory_blocks.at(result_params.filename),
              record_count, result_params.filename,
              result_params.stream_specifications, result_params.stream_index);
        } else {
          std::string filename = result_params.filename;
          if (filename.back() != 'v') {
            filename += ".csv";
          }
          WriteResults(
              data_manager, table_memory_blocks.at(result_params.filename),
              record_count, filename, result_params.stream_specifications,
              result_params.stream_index);
        }
      }
    }
  }
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
  initialisation_sum_ +=
      std::chrono::duration_cast<std::chrono::microseconds>(init_end - begin)
          .count();
  /*if (is_last_) {
    std::cout << "INITIALISATION: " << initialisation_sum << std::endl;
  }*/
  Log(LogLevel::kTrace, "Running query!");
  auto result_sizes = fpga_manager->RunQueryAcceleration();
  Log(LogLevel::kTrace, "Query done!");

  std::chrono::steady_clock::time_point total_end =
      std::chrono::steady_clock::now();
  /*std::cout << "TOTAL EXEC:"
            << std::chrono::duration_cast<std::chrono::microseconds>(total_end -
                                                                     begin)
                   .count()
            << std::endl;*/
  Log(LogLevel::kInfo,
      "Init and run time = " +
          std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
                             total_end - begin)
                             .count()) +
          "[microseconds]");

  /*ProcessResults(data_manager, result_sizes, result_parameters,
                 table_memory_blocks, scheduling_table_data);*/
  std::vector<std::string> removable_tables;
  for (const auto& [table_name, counter] : table_counter) {
    if (counter < 0) {
      throw std::runtime_error("Incorrect table counting!");
    }
    if (counter == 0) {
      removable_tables.push_back(table_name);
    } else {
      CropSortedStatus(scheduling_table_data, table_name);
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
  const int begin_offset = 0;
  const int end_offset = 1;
  const int size_offset = 2;
  const int sequence_count = 3;

  // We crop only assuming that there was a filter or a join before a sorter -
  // Therefore there was less to sort and we have to crop.
  auto& current_data = scheduling_table_data.at(filename);
  if (!current_data.sorted_status.empty()) {
    if (current_data.sorted_status.size() != 4) {
      throw std::runtime_error(
          "Cropping odd sorted sequence structures not supported currently!");
    }  // Assuming the sorted status is correct.
    if (current_data.sorted_status.at(end_offset) + 1 >
        current_data.record_count) {
      current_data.sorted_status[end_offset] =
          std::max(current_data.record_count - 1, 0);
      current_data.sorted_status[sequence_count] =
          (current_data.record_count + current_data.sorted_status[size_offset] -
           1) /
          current_data.sorted_status[size_offset];
      // Check if it got cropped so much that it got sorted.
      if (current_data.sorted_status[sequence_count] == 1) {
        current_data.sorted_status[size_offset] = current_data.record_count;
      }
    }  // Else it's all fine!
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
      // For merge sort it is only 1 input - Give min module size for
      // calculating buffer size.
      input_params[0].smallest_module_size = *std::min_element(
          operation_params.at(1).begin(), operation_params.at(1).end());
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
