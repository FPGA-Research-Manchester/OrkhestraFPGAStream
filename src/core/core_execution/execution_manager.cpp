﻿/*
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

#include "execution_manager.hpp"

#include <algorithm>
#include <iostream>
#include <memory>

#include "query_scheduling_helper.hpp"

using orkhestrafs::core::core_execution::ExecutionManager;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;

void ExecutionManager::SetFinishedFlag() { busy_flag_ = false; }

void ExecutionManager::Execute(
    std::unique_ptr<ExecutionPlanGraphInterface> execution_graph) {
  unscheduled_graph_ = std::move(execution_graph);

  busy_flag_ = true;
  while (busy_flag_) {
    auto new_state = current_state_->Execute(this);
    if (new_state) {
      current_state_ = std::move(new_state);
    }
  }
}

auto ExecutionManager::IsUnscheduledNodesGraphEmpty() -> bool {
  return unscheduled_graph_->IsEmpty();
}
auto ExecutionManager::IsARunScheduled() -> bool {
  return !query_node_runs_queue_.empty();
}
auto ExecutionManager::IsRunReadyForExecution() -> bool {
  return !query_nodes_.empty();
}

void ExecutionManager::ScheduleUnscheduledNodes() {
  query_node_runs_queue_ = query_manager_->ScheduleNextSetOfNodes(
      unscheduled_graph_->ExportRootNodes(), nodes_constrained_to_first_,
      current_available_nodes_, processed_nodes_, current_query_graph_,
      current_tables_metadata_, *accelerator_library_, config_, *scheduler_);

//  auto nodes_and_links = query_manager_->ScheduleUnscheduledNodes(
//      unscheduled_graph_->ExportRootNodes(), config_, *scheduler_);
//  all_reuse_links_ = nodes_and_links.first;
//  query_node_runs_queue_ = nodes_and_links.second;
}
void ExecutionManager::SetupNextRunData() {
  query_manager_->LoadNextBitstreamIfNew(
      memory_manager_.get(),
      config_.accelerator_library.at(query_node_runs_queue_.front().first),
      config_);
  auto next_scheduled_run_nodes = PopNextScheduledRun();

  for (const auto& node : next_scheduled_run_nodes) {
    scheduled_node_names_.push_back(node->node_name);
  }

  current_reuse_links_ = query_manager_->GetCurrentLinks(
      next_scheduled_run_nodes, all_reuse_links_);

  auto execution_nodes_and_result_params =
      query_manager_->SetupAccelerationNodesForExecution(
          data_manager_.get(), memory_manager_.get(),
          accelerator_library_.get(), input_memory_blocks_,
          output_memory_blocks_, input_stream_sizes_, output_stream_sizes_,
          next_scheduled_run_nodes);
  query_nodes_ = std::move(execution_nodes_and_result_params.first);
  result_parameters_ = std::move(execution_nodes_and_result_params.second);
}
void ExecutionManager::ExecuteAndProcessResults() {
  query_manager_->ExecuteAndProcessResults(
      fpga_manager_.get(), data_manager_.get(), output_memory_blocks_,
      output_stream_sizes_, result_parameters_, query_nodes_);
  query_manager_->FreeMemoryBlocks(memory_manager_.get(), input_memory_blocks_,
                                   output_memory_blocks_, input_stream_sizes_,
                                   output_stream_sizes_, current_reuse_links_,
                                   scheduled_node_names_);
}
auto ExecutionManager::IsRunValid() -> bool {
  return query_manager_->IsRunValid(query_nodes_);
}

auto ExecutionManager::PopNextScheduledRun()
    -> std::vector<std::shared_ptr<QueryNode>> {
  const auto executable_query_nodes = query_node_runs_queue_.front().second;
  query_node_runs_queue_.pop();
  return executable_query_nodes;
}

void ExecutionManager::PopAndPrintCurrentPlan() {
  int run_count = 0;
  while (!query_node_runs_queue_.empty()) {
    auto executable_query_nodes = query_node_runs_queue_.front().second;
    std::cout << "Run " << run_count << std::endl;
    for (const auto& node : executable_query_nodes) {
      std::cout << node->node_name << " ";
    }
    std::cout << std::endl;
    run_count++;
    query_node_runs_queue_.pop();
  }
}

void ExecutionManager::SetupSchedulingData() {
  current_tables_metadata_ = config_.initial_all_tables_metadata;

  for (const auto& node : unscheduled_graph_->GetRootNodesPtrs()) {
    current_available_nodes_.push_back(node->node_name);
  }

  SetupSchedulingGraphAndConstrainedNodes(
      unscheduled_graph_->GetAllNodesPtrs(), current_query_graph_,
      *accelerator_library_, nodes_constrained_to_first_);
}

void ExecutionManager::SetupSchedulingGraphAndConstrainedNodes(
    const std::vector<QueryNode*>& all_query_nodes,
    std::map<std::string, SchedulingQueryNode>& current_scheduling_graph,
    AcceleratorLibraryInterface& hw_library,
    std::vector<std::string>& constrained_nodes_vector) {
  for (const auto& node : all_query_nodes) {
    AddSchedulingNodeToGraph(node, current_scheduling_graph, hw_library);

    AddSavedNodesToConstrainedList(node, constrained_nodes_vector);
    AddFirstModuleNodesToConstrainedList(node, constrained_nodes_vector,
                                         hw_library);
  }
  AddSplittingNodesToConstrainedList(current_scheduling_graph,
                                     constrained_nodes_vector);
}

void ExecutionManager::AddSavedNodesToConstrainedList(
    QueryNode* const& node, std::vector<std::string>& constrained_nodes) {
  for (int node_index = 0; node_index < node->is_checked.size(); node_index++) {
    if (node->is_checked[node_index]) {
      auto constrained_node_name = node->next_nodes[node_index]->node_name;
      if (std::find(constrained_nodes.begin(), constrained_nodes.end(),
                    constrained_node_name) == constrained_nodes.end()) {
        constrained_nodes.push_back(constrained_node_name);
      }
    }
  }
}

void ExecutionManager::AddSchedulingNodeToGraph(
    QueryNode* const& node,
    std::map<std::string, SchedulingQueryNode>& scheduling_graph,
    AcceleratorLibraryInterface& accelerator_library) {
  SchedulingQueryNode current_node;
  for (const auto& next_node : node->next_nodes) {
    if (next_node) {
      current_node.after_nodes.push_back(next_node->node_name);
    }
  }
  for (const auto& previous_node : node->previous_nodes) {
    auto previous_node_ptr = previous_node.lock();
    if (previous_node_ptr) {
      current_node.before_nodes.push_back(
          {previous_node_ptr->node_name,
           QuerySchedulingHelper::FindNodePtrIndex(node,
                                                   previous_node_ptr.get())});
    }
  }
  for (const auto& input_files : node->input_data_definition_files) {
    current_node.data_tables.push_back(input_files);
  }
  current_node.corresponding_node = node;
  current_node.operation = node->operation_type;
  current_node.capacity = accelerator_library.GetNodeCapacity(
      node->operation_type, node->operation_parameters.operation_parameters);
  scheduling_graph.insert({node->node_name, current_node});
}

void ExecutionManager::AddFirstModuleNodesToConstrainedList(
    QueryNode* const& node, std::vector<std::string>& constrained_nodes,
    AcceleratorLibraryInterface& accelerator_library) {
  if (accelerator_library.IsNodeConstrainedToFirstInPipeline(
          node->operation_type) &&
      std::find(constrained_nodes.begin(), constrained_nodes.end(),
                node->node_name) == constrained_nodes.end()) {
    constrained_nodes.push_back(node->node_name);
  };
}

void ExecutionManager::AddSplittingNodesToConstrainedList(
    std::map<std::string, SchedulingQueryNode>& scheduling_graph,
    std::vector<std::string>& constrained_nodes) {
  std::vector<std::pair<std::string, int>> all_stream_dependencies;
  std::vector<std::pair<std::string, int>> splitting_streams;
  for (const auto& [node_name, node_parameters] : scheduling_graph) {
    for (const auto& previous_node_stream : node_parameters.before_nodes) {
      if (std::find(all_stream_dependencies.begin(),
                    all_stream_dependencies.end(),
                    previous_node_stream) == all_stream_dependencies.end()) {
        all_stream_dependencies.push_back(previous_node_stream);
      } else if (std::find(splitting_streams.begin(), splitting_streams.end(),
                           previous_node_stream) == splitting_streams.end()) {
        splitting_streams.push_back(previous_node_stream);
      }
    }
  }
  for (const auto& splitting_node_stream : splitting_streams) {
    for (const auto& potentially_constrained_node_name :
         scheduling_graph.at(splitting_node_stream.first).after_nodes) {
      auto before_nodes_vector =
          scheduling_graph.at(potentially_constrained_node_name).before_nodes;
      if (std::find(before_nodes_vector.begin(), before_nodes_vector.end(),
                    splitting_node_stream) != before_nodes_vector.end() &&
          std::find(constrained_nodes.begin(), constrained_nodes.end(),
                    potentially_constrained_node_name) ==
              constrained_nodes.end()) {
        constrained_nodes.push_back(potentially_constrained_node_name);
      }
    }
  }
}
