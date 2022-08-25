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
#include "scheduled_module.hpp"

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
  virtual void SetFinishedFlag() = 0;
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
  // virtual auto IsRunValid() -> bool = 0;
  /**
   * @brief Execute the next run ready for execution.
   */
  virtual void ExecuteAndProcessResults() = 0;
  /**
   * @brief Debug method to print the current stats. Will delete the run!
   */
  virtual void PrintCurrentStats() = 0;
  /**
   * @brief Method to setup current table data from config which tells the
   * scheduler about the available tables and query graphs.
   * @param Boolean for confirming the use of bitstreams
   */
  virtual void SetupSchedulingData(bool setup_bitstreams) = 0;

  /**
   * @brief Schedule unscheduled nodes.
   */
  virtual void BenchmarkScheduleUnscheduledNodes() = 0;
  /**
   * @brief Check if there are nodes to schedule.
   * @return Boolean flag showing if there are no more nodes to schedule.
   */
  virtual auto IsBenchmarkDone() -> bool = 0;

  virtual void UpdateAvailableNodesGraph() = 0;

  virtual auto GetFPGASpeed() -> int = 0;
  virtual void PrintHWState() = 0;
  virtual void LoadStaticBitstream() = 0;
  virtual void SetClockSpeed(int new_clock_speed) = 0;
  virtual void ChangeSchedulingTimeLimit(double new_time_limit) = 0;
  virtual void ChangeExecutionTimeLimit(int new_time_limit) = 0;
  virtual void SetInteractive(bool is_interactive) = 0;
  virtual auto IsInteractive() -> bool = 0;
  virtual void LoadStaticTables() = 0;
  virtual void AddNewNodes(std::string graph_filename) = 0;
  virtual void SetStartTimer() = 0;
  virtual void PrintExecTime() = 0;
  virtual void SetHWPrint(bool print_hw) = 0;
  virtual auto GetCurrentHW() -> std::map<QueryOperationType, OperationPRModules> = 0;
  virtual void LoadBitstream(ScheduledModule new_module) = 0;
};
}  // namespace orkhestrafs::dbmstodspi