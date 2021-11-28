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

using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;

class MockGraphProcessingFSM : public GraphProcessingFSMInterface {
 public:
  MOCK_METHOD(void, SetFinishedFlag, (), (override));
  MOCK_METHOD(bool, IsUnscheduledNodesGraphEmpty, (), (override));
  MOCK_METHOD(void, ScheduleUnscheduledNodes, (), (override));
  MOCK_METHOD(bool, IsARunScheduled, (), (override));
  MOCK_METHOD(void, SetupNextRunData, (), (override));
  MOCK_METHOD(bool, IsRunReadyForExecution, (), (override));
  //MOCK_METHOD(bool, IsRunValid, (), (override));
  MOCK_METHOD(void, ExecuteAndProcessResults, (), (override));
  MOCK_METHOD(void, PopAndPrintCurrentPlan, (), (override));
  MOCK_METHOD(void, SetupSchedulingData, (), (override));
};