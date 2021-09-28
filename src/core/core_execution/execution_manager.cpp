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

#include <memory>

using orkhestrafs::core::core_execution::ExecutionManager;

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
  auto nodes_and_links = query_manager_->ScheduleUnscheduledNodes(
      unscheduled_graph_->ExportRootNodes(), config_);
  all_reuse_links_ = nodes_and_links.first;
  query_node_runs_queue_ = nodes_and_links.second;
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
          data_manager_.get(), memory_manager_.get(), input_memory_blocks_,
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
