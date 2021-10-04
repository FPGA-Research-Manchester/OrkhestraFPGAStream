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

#include "setup_nodes_state.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "execute_state.hpp"
#include "mock_graph_processing_fsm.hpp"
#include "schedule_state.hpp"

using orkhestrafs::dbmstodspi::ExecuteState;
using orkhestrafs::dbmstodspi::ScheduleState;
using orkhestrafs::dbmstodspi::SetupNodesState;

namespace {

class SetupNodesStateTest : public ::testing::Test {
 protected:
  MockGraphProcessingFSM mock_fsm_;
  SetupNodesState state_under_test_;
};

TEST_F(SetupNodesStateTest, ExecuteGoesToScheduleStateWithNoNodes) {
  EXPECT_CALL(mock_fsm_, IsARunScheduled()).WillOnce(testing::Return(false));
  EXPECT_CALL(mock_fsm_, SetupNextRunData()).Times(0);

  ASSERT_EQ(typeid(*std::make_unique<ScheduleState>()).hash_code(),
            typeid(*state_under_test_.Execute(&mock_fsm_)).hash_code());
}

TEST_F(SetupNodesStateTest, ExecuteSetupsNodes) {
  EXPECT_CALL(mock_fsm_, IsARunScheduled()).WillOnce(testing::Return(true));
  EXPECT_CALL(mock_fsm_, SetupNextRunData()).Times(1);

  ASSERT_EQ(typeid(*std::make_unique<ExecuteState>()).hash_code(),
            typeid(*state_under_test_.Execute(&mock_fsm_)).hash_code());
}

}  // namespace