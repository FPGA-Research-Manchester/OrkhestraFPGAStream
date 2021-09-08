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

#include "schedule_state.hpp"

#include "node_scheduler.hpp"
#include "setup_nodes_state.hpp"
#include "logger.hpp"

using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::NodeScheduler;
using easydspi::dbmstodspi::ScheduleState;
using easydspi::dbmstodspi::SetupNodesState;
using easydspi::dbmstodspi::StateInterface;
using easydspi::dbmstodspi::logging::Log;
using easydspi::dbmstodspi::logging::LogLevel;

std::unique_ptr<StateInterface> ScheduleState::Execute(
    GraphProcessingFSMInterface* fsm) {
  Log(LogLevel::kTrace, "Schedule state");
  std::map<std::string, std::map<int, MemoryReuseTargets>> all_reuse_links;
  if (fsm->IsUnscheduledNodesGraphEmpty()) {
    fsm->setFinishedFlag();
    return std::make_unique<ScheduleState>();
  }
  fsm->ScheduleUnscheduledNodes();

  return std::make_unique<SetupNodesState>();
}