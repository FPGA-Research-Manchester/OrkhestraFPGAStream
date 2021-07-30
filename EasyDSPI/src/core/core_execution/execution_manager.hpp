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

#pragma once

#include <string>

#include "execution_manager_interface.hpp"
#include "graph_processing_fsm_interface.hpp"
#include "state_interface.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionManagerInterface;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::StateInterface;

namespace easydspi::core::core_execution {

class ExecutionManager : public ExecutionManagerInterface,
                         public GraphProcessingFSMInterface {
 private:
  std::unique_ptr<ExecutionPlanGraphInterface> initial_graph_;
  std::string current_graph_data_;
  Config initial_config_;
  Config current_config_;

  std::unique_ptr<StateInterface> current_state_;
  bool busy_flag_ = false;

 public:
  ~ExecutionManager() override = default;
  ExecutionManager(std::unique_ptr<StateInterface> start_state)
      : current_state_{std::move(start_state)} {}

  void setFinishedFlag() override;

  std::vector<std::string> execute(
      std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
          execution_input) override;
  std::string getCurrentGraphData() override;
  void setCurrentGraphData(std::string new_data) override;
  const Config& getCurrentConfig() override;
  void setCurrentConfig(const Config& new_config) override;
  const ExecutionPlanGraphInterface* getInitialGraph() override;
  const Config& getInitialConfig() override;
};
}  // namespace easydspi::core::core_execution