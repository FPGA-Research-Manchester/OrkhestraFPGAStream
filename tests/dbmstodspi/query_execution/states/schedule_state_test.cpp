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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_graph_processing_fsm.hpp"
#include "setup_nodes_state.hpp"

using orkhestrafs::dbmstodspi::ScheduleState;
using orkhestrafs::dbmstodspi::SetupNodesState;

namespace {

class ScheduleStateTest : public ::testing::Test {
 protected:
  MockGraphProcessingFSM mock_fsm_;
  ScheduleState state_under_test_;
};

TEST_F(ScheduleStateTest, ExecuteStopsFSM) {
  EXPECT_CALL(mock_fsm_, IsUnscheduledNodesGraphEmpty())
      .WillOnce(testing::Return(true));
  EXPECT_CALL(mock_fsm_, SetFinishedFlag()).Times(1);
  EXPECT_CALL(mock_fsm_, ScheduleUnscheduledNodes()).Times(0);

  ASSERT_EQ(typeid(*std::make_unique<ScheduleState>()).hash_code(),
            typeid(*state_under_test_.Execute(&mock_fsm_)).hash_code());
}

TEST_F(ScheduleStateTest, ExecuteSetupsNodes) {
  EXPECT_CALL(mock_fsm_, IsUnscheduledNodesGraphEmpty())
      .WillOnce(testing::Return(false));
  EXPECT_CALL(mock_fsm_, SetFinishedFlag()).Times(0);
  EXPECT_CALL(mock_fsm_, ScheduleUnscheduledNodes()).Times(1);

  ASSERT_EQ(typeid(*std::make_unique<SetupNodesState>()).hash_code(),
            typeid(*state_under_test_.Execute(&mock_fsm_)).hash_code());
}

}  // namespace