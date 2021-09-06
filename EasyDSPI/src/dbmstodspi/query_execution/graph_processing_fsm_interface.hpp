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
#include "fpga_manager_interface.hpp"
#include "memory_block_interface.hpp"
#include "query_scheduling_data.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using easydspi::core_interfaces::query_scheduling_data::MemoryReuseTargets;
using easydspi::core_interfaces::query_scheduling_data::QueryNode;
using easydspi::core_interfaces::query_scheduling_data::RecordSizeAndCount;
using easydspi::core_interfaces::query_scheduling_data::StreamResultParameters;

namespace easydspi::dbmstodspi {

class GraphProcessingFSMInterface {
 private:
 public:
  virtual ~GraphProcessingFSMInterface() = default;

  virtual void setFinishedFlag() = 0;

  virtual auto IsUnscheduledNodesGraphEmpty() -> bool = 0;
  virtual void ScheduleUnscheduledNodes() = 0;
  virtual auto IsARunScheduled() -> bool = 0;
  virtual void SetupNextRunData() = 0;
  virtual auto IsRunReadyForExecution() -> bool = 0;
  virtual auto IsRunValid() -> bool = 0;
  virtual void ExecuteAndProcessResults() = 0;
};
}  // namespace easydspi::dbmstodspi