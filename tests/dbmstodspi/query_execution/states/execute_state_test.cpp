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

#include "execute_state.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_graph_processing_fsm.hpp"
#include "setup_nodes_state.hpp"

using orkhestrafs::dbmstodspi::ExecuteState;
using orkhestrafs::dbmstodspi::SetupNodesState;

namespace {

class ExecuteStateTest : public ::testing::Test {};

TEST_F(ExecuteStateTest, ExecuteProcessesResultsAndReturnsSetupState) {
  MockGraphProcessingFSM mock_fsm;

  EXPECT_CALL(mock_fsm, IsRunReadyForExecution())
      .WillOnce(testing::Return(true));
  // EXPECT_CALL(mock_fsm, IsRunValid()).WillOnce(testing::Return(true));
  EXPECT_CALL(mock_fsm, ExecuteAndProcessResults()).Times(1);

  ExecuteState state_under_test;
  ASSERT_EQ(typeid(*std::make_unique<SetupNodesState>()).hash_code(),
            typeid(*state_under_test.Execute(&mock_fsm)).hash_code());
}

TEST_F(ExecuteStateTest, DISABLED_ExecuteCrashesWithInvalidRun) {
  MockGraphProcessingFSM mock_fsm;

  EXPECT_CALL(mock_fsm, IsRunReadyForExecution())
      .WillOnce(testing::Return(true));
  // EXPECT_CALL(mock_fsm, IsRunValid()).WillOnce(testing::Return(false));
  EXPECT_CALL(mock_fsm, ExecuteAndProcessResults()).Times(0);

  ExecuteState state_under_test;
  ASSERT_THROW(state_under_test.Execute(&mock_fsm), std::runtime_error);
}

TEST_F(ExecuteStateTest, ExecuteCrashesWithNoNodes) {
  MockGraphProcessingFSM mock_fsm;

  EXPECT_CALL(mock_fsm, IsRunReadyForExecution())
      .WillOnce(testing::Return(false));
  // EXPECT_CALL(mock_fsm, IsRunValid()).Times(0);
  EXPECT_CALL(mock_fsm, ExecuteAndProcessResults()).Times(0);

  ExecuteState state_under_test;
  ASSERT_THROW(state_under_test.Execute(&mock_fsm), std::runtime_error);
}

}  // namespace