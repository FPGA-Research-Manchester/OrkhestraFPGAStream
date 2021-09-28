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

#include "execution_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "fpga_manager_factory.hpp"
#include "mock_data_manager.hpp"
#include "mock_graph.hpp"
#include "mock_memory_manager.hpp"
#include "mock_query_manager.hpp"
#include "mock_state.hpp"
#include "stream_data_parameters.hpp"

namespace {

using orkhestrafs::core::core_execution::ExecutionManager;
using orkhestrafs::dbmstodspi::StreamDataParameters;
using testing::_;

class ExecutionManagerTest : public ::testing::Test {
 protected:
  ExecutionManagerTest()
      : mock_query_manager_ptr_(std::make_unique<MockQueryManager>()),
        mock_data_manager_ptr_(std::make_unique<MockDataManager>()),
        mock_memory_manager_ptr_(std::make_unique<MockMemoryManager>()),
        mock_first_state_ptr_(std::make_unique<MockState>()),
        mock_second_state_ptr_(std::make_unique<MockState>()),
        mock_query_manager_(*mock_query_manager_ptr_),
        mock_data_manager_(*mock_data_manager_ptr_),
        mock_memory_manager_(*mock_memory_manager_ptr_),
        mock_first_state_(*mock_first_state_ptr_),
        mock_second_state_(*mock_second_state_ptr_) {}

  std::unique_ptr<MockQueryManager> mock_query_manager_ptr_;
  std::unique_ptr<MockDataManager> mock_data_manager_ptr_;
  std::unique_ptr<MockMemoryManager> mock_memory_manager_ptr_;
  std::unique_ptr<MockState> mock_first_state_ptr_;
  std::unique_ptr<MockState> mock_second_state_ptr_;

  MockQueryManager& mock_query_manager_;
  MockDataManager& mock_data_manager_;
  MockMemoryManager& mock_memory_manager_;
  MockState& mock_first_state_;
  MockState& mock_second_state_;

  Config test_config_;
};

auto StopFSM(GraphProcessingFSMInterface* ptr)
    -> std::unique_ptr<StateInterface> {
  ptr->SetFinishedFlag();
  return std::make_unique<MockState>();
}

// Call execute on two mock nodes.
// One of them calls finish.
TEST_F(ExecutionManagerTest, ExecuteFinishesAfterTwoStates) {
  ExecutionManager execution_manager_under_test(
      test_config_, std::move(mock_query_manager_ptr_),
      std::move(mock_data_manager_ptr_), std::move(mock_memory_manager_ptr_),
      std::move(mock_first_state_ptr_));
  EXPECT_CALL(mock_first_state_, Execute(&execution_manager_under_test))
      .WillOnce(
          testing::Return(testing::ByMove(std::move(mock_second_state_ptr_))));

  EXPECT_CALL(mock_second_state_, Execute(&execution_manager_under_test))
      .WillOnce(testing::Invoke(StopFSM));

  std::unique_ptr<MockGraph> mock_graph_ptr = std::make_unique<MockGraph>();

  execution_manager_under_test.Execute(std::move(mock_graph_ptr));
}

TEST_F(ExecutionManagerTest, IsUnscheduledNodesGraphEmptyUsesGraph) {
  ExecutionManager execution_manager_under_test(
      test_config_, std::move(mock_query_manager_ptr_),
      std::move(mock_data_manager_ptr_), std::move(mock_memory_manager_ptr_),
      std::move(mock_first_state_ptr_));

  EXPECT_CALL(mock_first_state_, Execute(&execution_manager_under_test))
      .WillOnce(testing::Invoke(StopFSM));

  std::unique_ptr<MockGraph> mock_graph_ptr = std::make_unique<MockGraph>();
  MockGraph& mock_graph = *mock_graph_ptr;

  EXPECT_CALL(mock_graph, IsEmpty())
      .WillOnce(testing::Return(true))
      .WillOnce(testing::Return(false));

  execution_manager_under_test.Execute(std::move(mock_graph_ptr));
  ASSERT_TRUE(execution_manager_under_test.IsUnscheduledNodesGraphEmpty());
  ASSERT_FALSE(execution_manager_under_test.IsUnscheduledNodesGraphEmpty());
}

// This one next
TEST_F(ExecutionManagerTest, ScheduleUnscheduledNodesUsesGraph) {
  ExecutionManager execution_manager_under_test(
      test_config_, std::move(mock_query_manager_ptr_),
      std::move(mock_data_manager_ptr_), std::move(mock_memory_manager_ptr_),
      std::move(mock_first_state_ptr_));

  EXPECT_CALL(mock_first_state_, Execute(&execution_manager_under_test))
      .WillOnce(testing::Invoke(StopFSM));

  std::unique_ptr<MockGraph> mock_graph_ptr = std::make_unique<MockGraph>();
  MockGraph& mock_graph = *mock_graph_ptr;

  // std::vector<std::shared_ptr<QueryNode>> expected_nodes;

  EXPECT_CALL(mock_graph, ExportRootNodes()).Times(1);
  EXPECT_CALL(mock_query_manager_, ScheduleUnscheduledNodes(_, _)).Times(1);

  execution_manager_under_test.Execute(std::move(mock_graph_ptr));
  execution_manager_under_test.ScheduleUnscheduledNodes();
}

// Difficult one
TEST_F(ExecutionManagerTest, SetupPopsScheduledNodes) {
  QueryOperationType expected_operation_type = QueryOperationType::kAddition;
  std::vector<QueryOperation> expected_operations = {
      {expected_operation_type, {}}};
  test_config_.accelerator_library.insert({expected_operations, "some_file"});
  ExecutionManager execution_manager_under_test(
      test_config_, std::move(mock_query_manager_ptr_),
      std::move(mock_data_manager_ptr_), std::move(mock_memory_manager_ptr_),
      std::move(mock_first_state_ptr_));
  EXPECT_CALL(mock_first_state_, Execute(&execution_manager_under_test))
      .WillOnce(testing::Invoke(StopFSM));

  std::unique_ptr<MockGraph> mock_graph_ptr = std::make_unique<MockGraph>();
  MockGraph& mock_graph = *mock_graph_ptr;

  std::queue<std::pair<ConfigurableModulesVector,
                       std::vector<std::shared_ptr<QueryNode>>>>
      query_node_runs_queue;
  std::vector<std::shared_ptr<QueryNode>> expected_nodes;
  query_node_runs_queue.push({expected_operations, expected_nodes});
  std::map<std::string, std::map<int, MemoryReuseTargets>> expected_links;
  std::pair<std::map<std::string, std::map<int, MemoryReuseTargets>>,
            std::queue<std::pair<ConfigurableModulesVector,
                                 std::vector<std::shared_ptr<QueryNode>>>>>
      schedule_results = {expected_links, query_node_runs_queue};

  std::vector<StreamDataParameters> stream_params;
  int operation_module_location = 0;
  std::vector<std::vector<int>> operation_parameters;

  AcceleratedQueryNode test_node = {
      stream_params, stream_params, expected_operation_type,
      operation_module_location, {}, operation_parameters};

  std::vector<AcceleratedQueryNode> expected_accel_nodes = {test_node};
  std::map<std::string, std::vector<StreamResultParameters>>
      expected_result_params;
  std::pair<std::vector<AcceleratedQueryNode>,
            std::map<std::string, std::vector<StreamResultParameters>>>
      expected_setup_results = {expected_accel_nodes, expected_result_params};

  EXPECT_CALL(mock_graph, ExportRootNodes()).Times(1);
  EXPECT_CALL(mock_query_manager_, ScheduleUnscheduledNodes(_, _))
      .WillOnce(testing::Return(schedule_results));

  EXPECT_CALL(mock_query_manager_, LoadNextBitstreamIfNew(_, _, _)).Times(1);
  EXPECT_CALL(mock_query_manager_,
              GetCurrentLinks(expected_nodes, expected_links))
      .Times(1);
  EXPECT_CALL(mock_query_manager_, SetupAccelerationNodesForExecution(
                                       _, _, _, _, _, _, expected_nodes))
      .WillOnce(testing::Return(expected_setup_results));

  execution_manager_under_test.Execute(std::move(mock_graph_ptr));
  EXPECT_FALSE(execution_manager_under_test.IsARunScheduled());
  EXPECT_FALSE(execution_manager_under_test.IsRunReadyForExecution());
  execution_manager_under_test.ScheduleUnscheduledNodes();
  EXPECT_TRUE(execution_manager_under_test.IsARunScheduled());
  execution_manager_under_test.SetupNextRunData();
  EXPECT_FALSE(execution_manager_under_test.IsARunScheduled());
  EXPECT_TRUE(execution_manager_under_test.IsRunReadyForExecution());
}

TEST_F(ExecutionManagerTest, ExecuteAccelerationNodesUsesQueryManager) {
  ExecutionManager execution_manager_under_test(
      test_config_, std::move(mock_query_manager_ptr_),
      std::move(mock_data_manager_ptr_), std::move(mock_memory_manager_ptr_),
      std::move(mock_first_state_ptr_));

  EXPECT_CALL(mock_query_manager_, ExecuteAndProcessResults(_, _, _, _, _, _))
      .Times(1);
  EXPECT_CALL(mock_query_manager_, FreeMemoryBlocks(_, _, _, _, _, _, _))
      .Times(1);

  execution_manager_under_test.ExecuteAndProcessResults();
}

TEST_F(ExecutionManagerTest, IsRunValidUsesQueryManager) {
  ExecutionManager execution_manager_under_test(
      test_config_, std::move(mock_query_manager_ptr_),
      std::move(mock_data_manager_ptr_), std::move(mock_memory_manager_ptr_),
      std::move(mock_first_state_ptr_));

  EXPECT_CALL(mock_query_manager_, IsRunValid(_))
      .WillOnce(testing::Return(true))
      .WillOnce(testing::Return(false));

  ASSERT_TRUE(execution_manager_under_test.IsRunValid());
  ASSERT_FALSE(execution_manager_under_test.IsRunValid());
}

}  // namespace