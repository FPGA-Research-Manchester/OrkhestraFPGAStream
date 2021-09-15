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

#include "elastic_module_checker.hpp"
#include "fpga_manager.hpp"
#include "id_manager.hpp"
#include "logger.hpp"
#include "node_scheduler.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_data_parameters.hpp"
#include "table_manager.hpp"
#include "util.hpp"

using orkhestrafs::dbmstodspi::QueryManager;
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

auto QueryManager::CreateFPGAManager(MemoryManagerInterface* memory_manager)
    -> std::unique_ptr<FPGAManagerInterface> {
  return std::make_unique<FPGAManager>(memory_manager);
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
    const std::vector<int>& stream_ids,
    const std::vector<std::vector<int>>& node_parameters,
    const std::vector<std::unique_ptr<MemoryBlockInterface>>&
        allocated_memory_blocks,
    const std::vector<RecordSizeAndCount>& stream_sizes)
    -> std::vector<StreamDataParameters> {
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

    StreamDataParameters current_stream_parameters = {
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

    auto input_params =
        CreateStreamParams(input_ids[node->node_name],
                           node->operation_parameters.input_stream_parameters,
                           input_memory_blocks[node->node_name],
                           input_stream_sizes[node->node_name]);
    auto output_params =
        CreateStreamParams(output_ids[node->node_name],
                           node->operation_parameters.output_stream_parameters,
                           output_memory_blocks[node->node_name],
                           output_stream_sizes[node->node_name]);

    query_nodes.push_back({std::move(input_params), std::move(output_params),
                           node->operation_type, node->module_location,
                           node->operation_parameters.operation_parameters});
    StoreStreamResultPrameters(result_parameters, output_ids[node->node_name],
                               *node, output_memory_blocks[node->node_name]);
  }
  return {query_nodes, result_parameters};
}

void QueryManager::LoadNextBitstreamIfNew(
    MemoryManagerInterface* memory_manager, std::string bitstream_file_name,
    Config config) {
  return memory_manager->LoadBitstreamIfNew(
      bitstream_file_name,
      config.required_memory_space.at(bitstream_file_name));
}

auto QueryManager::ScheduleUnscheduledNodes(
    std::vector<std::shared_ptr<QueryNode>> unscheduled_root_nodes,
    Config config)
    -> std::pair<
        std::map<std::string, std::map<int, MemoryReuseTargets>>,
        std::queue<std::pair<ConfigurableModulesVector,
                             std::vector<std::shared_ptr<QueryNode>>>>> {
  std::map<std::string, std::map<int, MemoryReuseTargets>> all_reuse_links;

  auto query_node_runs_queue = NodeScheduler::FindAcceleratedQueryNodeSets(
      std::move(unscheduled_root_nodes), config.accelerator_library,
      config.module_library, all_reuse_links);
  return {all_reuse_links, query_node_runs_queue};
}

auto QueryManager::IsRunValid(std::vector<AcceleratedQueryNode> current_run)
    -> bool {
  for (const auto& node : current_run) {
    if (!ElasticModuleChecker::IsRunValid(node.input_streams,
                                          node.operation_type,
                                          node.operation_parameters)) {
      return false;
    }
  }
  return true;
}

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
    const std::vector<AcceleratedQueryNode>& execution_query_nodes) {
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
}
