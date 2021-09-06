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

#include <iostream>

#include "query_manager.hpp"

#include "elastic_module_checker.hpp"
#include "fpga_manager.hpp"
#include "node_scheduler.hpp"

using easydspi::dbmstodspi::QueryManager;

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

  std::cout << "Links" << std::endl;

  return current_links;
}

auto QueryManager::CreateFPGAManager(MemoryManagerInterface* memory_manager)
    -> std::unique_ptr<FPGAManagerInterface> {

    std::cout << "FPGA Manager" << std::endl;

  return std::make_unique<FPGAManager>(memory_manager);
}

auto QueryManager::SetupAccelerationNodesForExecution(
    DataManagerInterface* data_manager,
    MemoryManagerInterface* memory_manager,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>*
        input_memory_blocks,
    std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>*
        output_memory_blocks,
    std::map<std::string, std::vector<RecordSizeAndCount>>* input_stream_sizes,
    std::map<std::string, std::vector<RecordSizeAndCount>>* output_stream_sizes,
    const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes)
    -> std::pair<std::vector<AcceleratedQueryNode>,
                 std::map<std::string, std::vector<StreamResultParameters>>> {
  std::map<std::string, std::vector<StreamResultParameters>> result_parameters;
  std::vector<AcceleratedQueryNode> query_nodes;

  std::map<std::string, std::vector<int>> output_ids;
  std::map<std::string, std::vector<int>> input_ids;

  // InitialiseVectorSizes(current_query_nodes, input_memory_blocks,
  //                      output_memory_blocks, input_stream_sizes,
  //                      output_stream_sizes);

  // IDManager::AllocateStreamIDs(CreateReferenceVector(current_query_nodes),
  //                             input_ids, output_ids);

  // for (const auto& node : current_query_nodes) {
  //  AllocateOutputMemoryBlocks(memory_manager, data_manager,
  //                             output_memory_blocks[node->node_name], *node,
  //                             output_stream_sizes[node->node_name]);
  //  AllocateInputMemoryBlocks(
  //      memory_manager, data_manager, input_memory_blocks[node->node_name],
  //      *node, output_stream_sizes, input_stream_sizes[node->node_name]);

  //  auto input_params =
  //      CreateStreamParams(input_ids[node->node_name],
  //                         node->operation_parameters.input_stream_parameters,
  //                         input_memory_blocks[node->node_name],
  //                         input_stream_sizes[node->node_name]);
  //  auto output_params =
  //      CreateStreamParams(output_ids[node->node_name],
  //                         node->operation_parameters.output_stream_parameters,
  //                         output_memory_blocks[node->node_name],
  //                         output_stream_sizes[node->node_name]);

  //  query_nodes.push_back({std::move(input_params), std::move(output_params),
  //                         node->operation_type, node->module_location,
  //                         node->operation_parameters.operation_parameters});
  //  StoreStreamResultPrameters(result_parameters, output_ids[node->node_name],
  //                             *node, output_memory_blocks[node->node_name]);
  //}

  std::cout << "Acceleration" << std::endl;

  return {query_nodes, result_parameters};
}

void QueryManager::LoadNextBitstreamIfNew(
    MemoryManagerInterface* memory_manager, std::string bitstream_file_name,
    Config config) {

    std::cout << "Bitstream" << std::endl;

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

    std::cout << "Scheduler" << std::endl;

  std::map<std::string, std::map<int, MemoryReuseTargets>> all_reuse_links;

  auto query_node_runs_queue = NodeScheduler::FindAcceleratedQueryNodeSets(
      std::move(unscheduled_root_nodes), config.accelerator_library,
      config.module_library, all_reuse_links);
  return {all_reuse_links, query_node_runs_queue};
}

auto QueryManager::IsRunValid(std::vector<AcceleratedQueryNode> current_run)
    -> bool {

    std::cout << "Check" << std::endl;

  for (const auto& node : current_run) {
    ElasticModuleChecker::CheckElasticityNeeds(
        node.input_streams, node.operation_type, node.operation_parameters);
  }

  return true;
}

void QueryManager::ExecuteAndProcessResults() {
  std::cout << "Execute" << std::endl;
}
