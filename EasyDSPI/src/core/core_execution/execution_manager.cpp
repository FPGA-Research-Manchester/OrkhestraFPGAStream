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

#include "fpga_manager.hpp"
#include "fpga_manager_interface.hpp"

using easydspi::core::core_execution::ExecutionManager;
using easydspi::dbmstodspi::FPGAManager;
using easydspi::dbmstodspi::FPGAManagerInterface;

void ExecutionManager::setFinishedFlag() { busy_flag_ = false; }

void ExecutionManager::execute(
    std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
        execution_input) {
  initial_config_ = execution_input.second;
  initial_graph_ = std::move(execution_input.first);

  busy_flag_ = true;
  while (busy_flag_) {
    auto new_state = current_state_->execute(this);
    if (new_state) {
      current_state_ = std::move(new_state);
    }
  }
}

void ExecutionManager::SetQueryNodeRunsQueue(
    const std::queue<std::pair<ConfigurableModulesVector,
                               std::vector<std::shared_ptr<QueryNode>>>>&
        new_queue) {
  query_node_runs_queue_ = new_queue;
}
auto ExecutionManager::GetConfig() -> Config { return initial_config_; }
auto ExecutionManager::GetReuseLinks()
    -> std::map<std::string, std::map<int, MemoryReuseTargets>> {
  return all_reuse_links_;
}
void ExecutionManager::SetReuseLinks(
    const std::map<std::string, std::map<int, MemoryReuseTargets>> new_links) {
  all_reuse_links_ = new_links;
}
