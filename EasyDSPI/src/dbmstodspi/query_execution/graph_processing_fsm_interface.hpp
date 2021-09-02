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

#include <memory>
#include <queue>

#include "config.hpp"
#include "execution_plan_graph_interface.hpp"
#include "query_scheduling_data.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using easydspi::core_interfaces::query_scheduling_data::QueryNode;
using easydspi::core_interfaces::query_scheduling_data::MemoryReuseTargets;

namespace easydspi::dbmstodspi {

class GraphProcessingFSMInterface {
 private:
 public:
  virtual ~GraphProcessingFSMInterface() = default;

  virtual void setFinishedFlag() = 0;

  virtual void SetQueryNodeRunsQueue(
      const std::queue<std::pair<ConfigurableModulesVector,
                           std::vector<std::shared_ptr<QueryNode>>>>& new_queue) = 0;
  virtual auto GetConfig() -> Config = 0;
  virtual auto GetReuseLinks()
      -> std::map<std::string, std::map<int, MemoryReuseTargets>> = 0;
  virtual void SetReuseLinks(
      const std::map<std::string, std::map<int, MemoryReuseTargets>> new_links) = 0;
};
}  // namespace easydspi::dbmstodspi