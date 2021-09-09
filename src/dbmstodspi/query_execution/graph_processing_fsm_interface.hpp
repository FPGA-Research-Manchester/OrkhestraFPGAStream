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

using orkhestrafs::core_interfaces::query_scheduling_data::
    ConfigurableModulesVector;
using orkhestrafs::core_interfaces::query_scheduling_data::MemoryReuseTargets;
using orkhestrafs::core_interfaces::query_scheduling_data::RecordSizeAndCount;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Interface to describe classes capable of running the FSM.
 */
class GraphProcessingFSMInterface {
 private:
 public:
  virtual ~GraphProcessingFSMInterface() = default;

  /**
   * @brief Stop the FSM form executing the next state.
   */
  virtual void setFinishedFlag() = 0;
  /**
   * @brief Check if there are nodes to schedule.
   * @return Boolean flag showing if there are no more nodes to schedule.
   */
  virtual auto IsUnscheduledNodesGraphEmpty() -> bool = 0;
  /**
   * @brief Schedule unscheduled nodes.
   */
  virtual void ScheduleUnscheduledNodes() = 0;
  /**
   * @brief Check if there is a run scheduled.
   * @return Boolean flag showing if a run has been scheduled.
   */
  virtual auto IsARunScheduled() -> bool = 0;
  /**
   * @brief Setup next scheduled run.
   */
  virtual void SetupNextRunData() = 0;
  /**
   * @brief Check if there is a run setup for execution.
   * @return Boolean flag to show if there is a run ready.
   */
  virtual auto IsRunReadyForExecution() -> bool = 0;
  /**
   * @brief Check if the next run doesn't violate any constraints. During setup
   * memory constraints could get violated.
   * @return Boolean flag showing if the run is valid.
   */
  virtual auto IsRunValid() -> bool = 0;
  /**
   * @brief Execute the next run ready for execution.
   */
  virtual void ExecuteAndProcessResults() = 0;
};
}  // namespace orkhestrafs::dbmstodspi