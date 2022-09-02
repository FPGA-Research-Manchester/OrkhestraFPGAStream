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

#include "gmock/gmock.h"
#include "graph_processing_fsm_interface.hpp"
#include "scheduled_module.hpp"

using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;
using orkhestrafs::dbmstodspi::ScheduledModule;

class MockGraphProcessingFSM : public GraphProcessingFSMInterface {

 using HWLibrary = std::map<QueryOperationType, OperationPRModules>;

 public:
  MOCK_METHOD(void, SetFinishedFlag, (), (override));
  MOCK_METHOD(bool, IsUnscheduledNodesGraphEmpty, (), (override));
  MOCK_METHOD(void, ScheduleUnscheduledNodes, (), (override));
  MOCK_METHOD(bool, IsARunScheduled, (), (override));
  MOCK_METHOD(void, SetupNextRunData, (), (override));
  MOCK_METHOD(bool, IsRunReadyForExecution, (), (override));
  // MOCK_METHOD(bool, IsRunValid, (), (override));
  MOCK_METHOD(void, ExecuteAndProcessResults, (), (override));
  MOCK_METHOD(void, PrintCurrentStats, (), (override));
  MOCK_METHOD(void, SetupSchedulingData, (bool), (override));

  MOCK_METHOD(bool, IsBenchmarkDone, (), (override));
  MOCK_METHOD(void, BenchmarkScheduleUnscheduledNodes, (), (override));
  MOCK_METHOD(void, UpdateAvailableNodesGraph, (), (override));

  MOCK_METHOD(int, GetFPGASpeed, (), (override));
  MOCK_METHOD(void, PrintHWState, (), (override));
  MOCK_METHOD(void, LoadStaticBitstream, (), (override));
  MOCK_METHOD(void, SetClockSpeed, (int new_clock_speed), (override));
  MOCK_METHOD(void, ChangeSchedulingTimeLimit, (double new_time_limit),
              (override));
  MOCK_METHOD(void, ChangeExecutionTimeLimit, (int new_time_limit), (override));
  MOCK_METHOD(void, SetInteractive, (bool is_interactive), (override));
  MOCK_METHOD(bool, IsInteractive, (), (override));
  MOCK_METHOD(void, LoadStaticTables, (), (override));
  MOCK_METHOD(void, AddNewNodes, (std::string graph_filename), (override));
  MOCK_METHOD(void, SetStartTimer, (), (override));
  MOCK_METHOD(void, PrintExecTime, (), (override));
  MOCK_METHOD(void, SetHWPrint, (bool print_hw), (override));
  MOCK_METHOD(HWLibrary, GetCurrentHW, (), (override));
  MOCK_METHOD(void, LoadBitstream, (ScheduledModule new_module), (override));
  MOCK_METHOD(bool, IsHWPrintEnabled, (), (override));
};