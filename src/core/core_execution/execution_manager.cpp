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
  return current_available_node_pointers_.empty();
  // TODO(Kaspar): Change the usage of the graph. Currently it's pointless.
  /*return unscheduled_graph_->IsEmpty();*/
}
auto ExecutionManager::IsARunScheduled() -> bool {
  return !query_node_runs_queue_.empty();
}
auto ExecutionManager::IsRunReadyForExecution() -> bool {
  return !query_nodes_.empty();
}

void ExecutionManager::ScheduleUnscheduledNodes() {
  query_node_runs_queue_ = query_manager_->ScheduleNextSetOfNodes(
      current_available_node_pointers_, nodes_constrained_to_first_,
      current_available_nodes_, processed_nodes_, current_query_graph_,
      current_tables_metadata_, *accelerator_library_, config_, *scheduler_,
      all_reuse_links_, current_configuration_);
}
void ExecutionManager::BenchmarkScheduleUnscheduledNodes() {
  query_manager_->BenchmarkScheduling(
      nodes_constrained_to_first_, current_available_nodes_, processed_nodes_,
      current_query_graph_, current_tables_metadata_, *accelerator_library_,
      config_, *scheduler_, current_configuration_);
}
auto ExecutionManager::IsBenchmarkDone() -> bool {
  return current_available_nodes_.empty();
}

void ExecutionManager::SetupNextRunData() {
  ConfigurableModulesVector next_set_of_operators;

  // const auto first = query_node_runs_queue_.front();  // getting the first
  // query_node_runs_queue_.pop();                       // removing him
  // query_node_runs_queue_.push(first);

  auto next_run = query_node_runs_queue_.front().first;
  for (auto& module_index : next_run) {
    next_set_of_operators.emplace_back(
        module_index.operation_type,
        config_.pr_hw_library.at(module_index.operation_type)
            .bitstream_map.at(module_index.bitstream)
            .capacity);
  }

  // if (config_.accelerator_library.find(next_set_of_operators) ==
  //    config_.accelerator_library.end()) {
  //  // for (const auto& thing : query_node_runs_queue_.front().first) {
  //  //  auto op_type = thing.operation_type;
  //  //  auto module_params = thing.resource_elasticity_data;
  //  //}
  //  throw std::runtime_error("Bitstream not found!");
  //}

  // Old
  /*query_manager_->LoadNextBitstreamIfNew(
      memory_manager_.get(),
      config_.accelerator_library.at(query_node_runs_queue_.front().first),
      config_);*/
  /*query_manager_->LoadInitialStaticBitstream(memory_manager_.get());
  query_manager_->LoadEmptyRoutingPRRegion(memory_manager_.get(),
                                           *accelerator_library_.get());*/
  auto [bitstreams_to_load, empty_modules] =
      query_manager_->GetPRBitstreamsToLoadWithPassthroughModules(
          current_configuration_, query_node_runs_queue_.front().first, 31);
  query_manager_->LoadPRBitstreams(memory_manager_.get(), bitstreams_to_load,
                                   *accelerator_library_);
  query_manager_->LoadPRBitstreams(memory_manager_.get(), bitstreams_to_load,
                                   *accelerator_library_);
  /*query_manager_->LoadPRBitstreams(memory_manager_.get(), bitstreams_to_load,
   *accelerator_library_.get());*/
  // Debugging
  // std::vector<std::pair<QueryOperationType, bool>> empty_modules;
  /*query_manager_->LoadEmptyRoutingPRRegion(memory_manager_.get(),
   *accelerator_library_.get());*/
  /*query_manager_->LoadPRBitstreams(memory_manager_.get(),config_.debug_forced_pr_bitstreams,
   *accelerator_library_.get()); exit(0);*/
  auto next_scheduled_run_nodes = PopNextScheduledRun();

  for (const auto& node : next_scheduled_run_nodes) {
    scheduled_node_names_.push_back(node->node_name);
  }

  current_reuse_links_ = query_manager_->GetCurrentLinks(all_reuse_links_);

  auto execution_nodes_and_result_params =
      query_manager_->SetupAccelerationNodesForExecution(
          data_manager_.get(), memory_manager_.get(),
          accelerator_library_.get(), input_memory_blocks_,
          output_memory_blocks_, input_stream_sizes_, output_stream_sizes_,
          next_scheduled_run_nodes);
  query_nodes_ = std::move(execution_nodes_and_result_params.first);
  for (int module_pos = 0; module_pos < empty_modules.size(); module_pos++) {
    if (empty_modules.at(module_pos).second) {
      query_nodes_.insert(
          query_nodes_.begin() + module_pos,
          accelerator_library_->GetEmptyModuleNode(
              empty_modules.at(module_pos).first, module_pos + 1));
    } else {
      query_nodes_.at(module_pos).operation_module_location = module_pos + 1;
    }
  }
  // For debugging - manual insertion of passthrough modules
  /*query_nodes_.insert(query_nodes_.begin(),
                      accelerator_library_->GetEmptyModuleNode(QueryOperationType::kMergeSort,
     2));*/
  /* query_nodes_.insert(
      query_nodes_.begin() + 2,
      accelerator_library_->GetEmptyModuleNode(QueryOperationType::kFilter,
     3));*/
  result_parameters_ = std::move(execution_nodes_and_result_params.second);
}
void ExecutionManager::ExecuteAndProcessResults() {
  query_manager_->ExecuteAndProcessResults(
      fpga_manager_.get(), data_manager_.get(), output_memory_blocks_,
      output_stream_sizes_, result_parameters_, query_nodes_,
      current_tables_metadata_, current_reuse_links_, current_query_graph_);
  query_manager_->FreeMemoryBlocks(memory_manager_.get(), input_memory_blocks_,
                                   output_memory_blocks_, input_stream_sizes_,
                                   output_stream_sizes_, current_reuse_links_,
                                   scheduled_node_names_);
}
// auto ExecutionManager::IsRunValid() -> bool {
//  return query_manager_->IsRunValid(query_nodes_);
//}

auto ExecutionManager::PopNextScheduledRun()
    -> std::vector<std::shared_ptr<QueryNode>> {
  const auto executable_query_nodes = query_node_runs_queue_.front().second;
  query_node_runs_queue_.pop();
  return executable_query_nodes;
}

void ExecutionManager::PrintCurrentStats() {
  query_manager_->PrintBenchmarkStats();
}

void ExecutionManager::SetupSchedulingData(bool setup_bitstreams) {
  for (const auto& node : unscheduled_graph_->GetRootNodesPtrs()) {
    current_available_nodes_.insert(node->node_name);
  }

  current_tables_metadata_ = config_.initial_all_tables_metadata;

  SetupSchedulingGraphAndConstrainedNodes(
      unscheduled_graph_->GetAllNodesPtrs(), current_query_graph_,
      *accelerator_library_, nodes_constrained_to_first_);

  if (setup_bitstreams) {
    // TODO(Kaspar): The static bitstream loading should be moved to a different
    // state!
    query_manager_->LoadInitialStaticBitstream(memory_manager_.get());
    // The empty routing should be part of the static really.
    query_manager_->LoadEmptyRoutingPRRegion(memory_manager_.get(),
                                             *accelerator_library_);
    current_available_node_pointers_ = unscheduled_graph_->ExportRootNodes();
  }
}

void ExecutionManager::SetupSchedulingGraphAndConstrainedNodes(
    const std::vector<QueryNode*>& all_query_nodes,
    std::unordered_map<std::string, SchedulingQueryNode>&
        current_scheduling_graph,
    AcceleratorLibraryInterface& hw_library,
    std::unordered_set<std::string>& constrained_nodes_vector) {
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
    QueryNode* const& node,
    std::unordered_set<std::string>& constrained_nodes) {
  for (int node_index = 0; node_index < node->is_checked.size(); node_index++) {
    if (node->is_checked[node_index]) {
      auto constrained_node_name = node->next_nodes[node_index]->node_name;
      if (constrained_nodes.find(constrained_node_name) ==
          constrained_nodes.end()) {
        constrained_nodes.insert(constrained_node_name);
      }
    }
  }
}

void ExecutionManager::AddSchedulingNodeToGraph(
    QueryNode* const& node,
    std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph,
    AcceleratorLibraryInterface& accelerator_library) {
  SchedulingQueryNode current_node;
  for (const auto& next_node : node->next_nodes) {
    if (next_node) {
      current_node.after_nodes.push_back(next_node->node_name);
    } else {
      current_node.after_nodes.emplace_back("");
    }
  }
  for (const auto& previous_node : node->previous_nodes) {
    auto previous_node_ptr = previous_node.lock();
    if (previous_node_ptr) {
      current_node.before_nodes.emplace_back(
          previous_node_ptr->node_name, QuerySchedulingHelper::FindNodePtrIndex(
                                            node, previous_node_ptr.get()));
    } else {
      current_node.before_nodes.emplace_back("", -1);
    }
  }
  for (const auto& input_files : node->input_data_definition_files) {
    current_node.data_tables.push_back(input_files);
  }
  current_node.operation = node->operation_type;
  current_node.capacity = accelerator_library.GetNodeCapacity(
      node->operation_type, node->operation_parameters.operation_parameters);
  scheduling_graph.insert({node->node_name, current_node});
}

void ExecutionManager::AddFirstModuleNodesToConstrainedList(
    QueryNode* const& node, std::unordered_set<std::string>& constrained_nodes,
    AcceleratorLibraryInterface& accelerator_library) {
  if (accelerator_library.IsNodeConstrainedToFirstInPipeline(
          node->operation_type)) {
    constrained_nodes.insert(node->node_name);
  };
}

// This check is looking at all the nodes if there are multiple identical before
// streams.
// TODO(Kaspar): Splitting nodes aren't supported at the moment anyway:
// after_nodes needs to be a vector of vectors!
void ExecutionManager::AddSplittingNodesToConstrainedList(
    std::unordered_map<std::string, SchedulingQueryNode>& scheduling_graph,
    std::unordered_set<std::string>& constrained_nodes) {
  // THere are no splitting nodes at the moment
  /*std::vector<std::pair<std::string, int>> all_stream_dependencies;
  std::vector<std::pair<std::string, int>> splitting_streams;*/

  /*for (const auto& [node_name, node_parameters] : scheduling_graph) {
    for (const auto& previous_node_stream : node_parameters.before_nodes) {
      if (previous_node_stream.second != -1) {
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
  }
  for (const auto& splitting_node_stream : splitting_streams) {
    for (const auto& potentially_constrained_node_name :
         scheduling_graph.at(splitting_node_stream.first).after_nodes) {
      if (!potentially_constrained_node_name.empty()) {
        auto before_nodes_vector =
            scheduling_graph.at(potentially_constrained_node_name).before_nodes;
        if (std::find(before_nodes_vector.begin(), before_nodes_vector.end(),
                      splitting_node_stream) != before_nodes_vector.end() &&
            std::find(constrained_nodes.begin(), constrained_nodes.end(),
                      potentially_constrained_node_name) ==
                constrained_nodes.end()) {
          constrained_nodes.insert(potentially_constrained_node_name);
        }
      }
    }
  }*/
}
