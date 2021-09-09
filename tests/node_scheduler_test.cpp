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

#include "node_scheduler.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>
/// Good documentation for using matchers:
/// https://github.com/google/googletest/blob/master/docs/reference/matchers.md
namespace {

using orkhestrafs::core_interfaces::query_scheduling_data::NodeOperationParameters;
using orkhestrafs::dbmstodspi::NodeScheduler;
using ModulesCombo =
    orkhestrafs::core_interfaces::query_scheduling_data::ConfigurableModulesVector;
using Node = orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;
using OpType = orkhestrafs::core_interfaces::operation_types::QueryOperationType;

const std::vector<std::string> kDefaultEmptyFileNames = {""};
const std::vector<std::string> kDefaultFileNames = {"some_name.csv"};
const std::vector<std::shared_ptr<Node>> kDefaultNextNodes;
const std::vector<std::weak_ptr<Node>> kDefaultPreviousNodes;
const std::vector<bool> kNoChecks = {false};

class NodeSchedulerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    base_parameters_ = {{{}}, {{}}, {{}}};

    base_supported_bitstreams_ = {
        {{{OpType::kMergeSort, {}}}, "Some_bitstream"},
        {{{OpType::kJoin, {}}}, "Some_bitstream"},
        {{{OpType::kLinearSort, {}}}, "Some_bitstream"},
        {{{OpType::kFilter, {}}}, "Some_bitstream"},
        {{{OpType::kPassThrough, {}}}, "Some_bitstream"}};
    base_existing_modules_ = {{OpType::kMergeSort, {}},
                              {OpType::kJoin, {}},
                              {OpType::kLinearSort, {}},
                              {OpType::kFilter, {}},
                              {OpType::kPassThrough, {}}};
    query_node_a_ = std::make_shared<Node>(
        kDefaultEmptyFileNames, kDefaultEmptyFileNames, OpType::kFilter,
        kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_, "A",
        kNoChecks);
    query_node_b_ = std::make_shared<Node>(
        kDefaultEmptyFileNames, kDefaultEmptyFileNames, OpType::kLinearSort,
        kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_, "B",
        kNoChecks);
    passthrough_node_ = std::make_shared<Node>(
        kDefaultEmptyFileNames, kDefaultEmptyFileNames, OpType::kPassThrough,
        kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_, "PASS",
        kNoChecks);
  }

  std::map<ModulesCombo, std::string> base_supported_bitstreams_;
  std::map<OpType, std::vector<std::vector<int>>> base_existing_modules_;
  std::map<std::string, std::map<int, std::vector<std::pair<std::string, int>>>>
      reuse_map_;

  std::shared_ptr<Node> passthrough_node_;
  // Two base nodes with a different operation
  std::shared_ptr<Node> query_node_a_;
  std::shared_ptr<Node> query_node_b_;

  NodeOperationParameters base_parameters_;
};

void CheckNodeFields(const std::vector<std::string>& input_files,
                     const std::vector<std::string>& output_files,
                     OpType operation,
                     const std::vector<std::shared_ptr<Node>>& next,
                     std::vector<std::weak_ptr<Node>> previous,
                     const NodeOperationParameters& parameters, int location,
                     const std::string& name, const std::vector<bool>& checks,
                     Node comparable_node) {
  EXPECT_THAT(comparable_node,
              testing::Field("input_files", &Node::input_data_definition_files,
                             input_files));
  EXPECT_THAT(
      comparable_node,
      testing::Field("output_files", &Node::output_data_definition_files,
                     output_files));
  EXPECT_THAT(comparable_node,
              testing::Field("operation", &Node::operation_type, operation));
  EXPECT_THAT(comparable_node, testing::Field("next", &Node::next_nodes, next));
  EXPECT_THAT(
      comparable_node,
      testing::Field("parameters", &Node::operation_parameters, parameters));
  EXPECT_THAT(comparable_node,
              testing::Field("location", &Node::module_location, location));
  EXPECT_THAT(comparable_node, testing::Field("name", &Node::node_name, name));
  EXPECT_THAT(comparable_node,
              testing::Field("checks", &Node::is_checked, checks));

  // EXPECT_THAT(comparable_node,
  //            testing::Field("previous", &Node::previous_nodes, previous));
  ASSERT_TRUE(std::equal(
      previous.begin(), previous.end(), comparable_node.previous_nodes.begin(),
      [](const std::weak_ptr<Node> l, const std::weak_ptr<Node> r) {
        return l.lock() == r.lock();
      }));
}

void CheckNodeEquality(Node real_node, const Node& expected_node) {
  CheckNodeFields(
      expected_node.input_data_definition_files,
      expected_node.output_data_definition_files, expected_node.operation_type,
      expected_node.next_nodes, expected_node.previous_nodes,
      expected_node.operation_parameters, expected_node.module_location,
      expected_node.node_name, expected_node.is_checked, std::move(real_node));
}

TEST_F(NodeSchedulerTest, MultipleAvailableNodesFindsCorrectNode) {
  auto expected_node = *query_node_a_;
  expected_node.module_location = 1;
  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_node);
}

TEST_F(NodeSchedulerTest, TwoNodesWereFoundWithDifferentRuns) {
  auto first_query = std::make_shared<Node>(
      kDefaultEmptyFileNames, kDefaultEmptyFileNames, OpType::kLinearSort,
      kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_, "1",
      kNoChecks);
  auto second_query = std::make_shared<Node>(
      kDefaultEmptyFileNames, kDefaultEmptyFileNames, OpType::kLinearSort,
      kDefaultNextNodes, kDefaultPreviousNodes, base_parameters_, "2",
      kNoChecks);

  auto expected_first_node = *first_query;
  auto expected_second_node = *second_query;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 1;
  ModulesCombo expected_module_vector = {{first_query->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {first_query,
                                                       second_query};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, TwoNodesWereFoundWithinTheSameRun) {
  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}},
                                         {query_node_b_->operation_type, {}}};

  base_supported_bitstreams_.insert({expected_module_vector, "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 2;
  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_,
                                                       query_node_b_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, PassthroughNodeIsUsedInTheSameRun) {
  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *passthrough_node_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = -1;

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_,
                                                       passthrough_node_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithDifferentRuns) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  // expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  // auto expected_filename = query_node_a_->node_name + "_0.csv";
  // expected_first_node.output_data_definition_files = {expected_filename};
  // expected_second_node.input_data_definition_files = {expected_filename};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, TwoPipelinedNodesWereFoundWithinTheSameRun) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}},
                                         {query_node_b_->operation_type, {}}};

  base_supported_bitstreams_.insert({expected_module_vector, "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 2;

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfInputProjection) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  query_node_b_->operation_parameters.input_stream_parameters = {
      {0, 1, 2, 3, 3}};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  // expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  // auto expected_filename = query_node_a_->node_name + "_0.csv";
  // expected_first_node.output_data_definition_files = {expected_filename};
  // expected_second_node.input_data_definition_files = {expected_filename};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfOutputChecking) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  query_node_a_->output_data_definition_files = kDefaultFileNames;
  query_node_a_->is_checked = {true};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  expected_second_node.input_data_definition_files = kDefaultFileNames;

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfOutputProjection) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  query_node_a_->operation_parameters.output_stream_parameters = {
      {0, 1, 2, 3, 3}};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  // expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  // auto expected_filename = query_node_a_->node_name + "_0.csv";
  // expected_first_node.output_data_definition_files = {expected_filename};
  // expected_second_node.input_data_definition_files = {expected_filename};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfIOProjection) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  query_node_a_->operation_parameters.output_stream_parameters = {
      {0, 1, 2, 3, 3}};
  query_node_b_->operation_parameters.input_stream_parameters = {
      {0, 1, 2, 3, 3}};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  // expected_first_node.next_nodes = {nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  // auto expected_filename = query_node_a_->node_name + "_0.csv";
  // expected_first_node.output_data_definition_files = {expected_filename};
  // expected_second_node.input_data_definition_files = {expected_filename};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, DifferentRunsBecauseOfSecondOutputProjection) {
  query_node_a_->next_nodes = {nullptr, query_node_b_};
  query_node_a_->output_data_definition_files = {"", ""};
  query_node_b_->previous_nodes = {query_node_a_};

  std::vector<int> some_data = {0, 1, 2, 3, 3};

  query_node_a_->operation_parameters.output_stream_parameters = {
      {},        some_data, some_data, some_data,
      some_data, some_data, some_data, some_data};

  base_supported_bitstreams_.insert({{{query_node_a_->operation_type, {}},
                                      {query_node_b_->operation_type, {}}},
                                     "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  // expected_first_node.next_nodes = {nullptr, nullptr};
  expected_second_node.module_location = 1;
  expected_second_node.previous_nodes = {std::weak_ptr<Node>()};

  expected_first_node.output_data_definition_files = {
      query_node_a_->node_name + "_0.csv", ""};

  // auto expected_filename = query_node_a_->node_name + "_1.csv";
  // expected_first_node.output_data_definition_files =
  // {query_node_a_->node_name + "_0.csv", expected_filename};
  // expected_second_node.input_data_definition_files = {expected_filename};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}}};
  ModulesCombo expected_second_module_vector = {
      {query_node_b_->operation_type, {}}};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  scheduling_results.pop();
  ASSERT_EQ(scheduling_results.front().first, expected_second_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, SameRunsDespiteIOProjection) {
  query_node_a_->next_nodes = {nullptr, query_node_b_};
  query_node_a_->output_data_definition_files = {"", ""};
  query_node_b_->previous_nodes = {query_node_a_};

  std::vector<int> some_data = {0, 1, 2, 3, 3};

  query_node_a_->operation_parameters.output_stream_parameters = {
      some_data, some_data, some_data, some_data,
      {},        some_data, some_data, some_data};
  query_node_a_->operation_parameters.input_stream_parameters = {
      {0, 1, 2, 3, 3}};
  query_node_b_->operation_parameters.output_stream_parameters = {
      {1, 2, 3, 4, 5, 6, 6}};
  query_node_b_->operation_parameters.input_stream_parameters = {
      {}, {12}, {1, 2, 3, 4}};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}},
                                         {query_node_b_->operation_type, {}}};

  base_supported_bitstreams_.insert({expected_module_vector, "Some_bitstream"});

  auto expected_first_node = *query_node_a_;
  auto expected_second_node = *query_node_b_;
  expected_first_node.module_location = 1;
  expected_second_node.module_location = 2;
  expected_first_node.output_data_definition_files = {
      query_node_a_->node_name + "_0.csv", ""};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(scheduling_results.front().first, expected_module_vector);
  CheckNodeEquality(*scheduling_results.front().second[0], expected_first_node);
  CheckNodeEquality(*scheduling_results.front().second[1],
                    expected_second_node);
}

TEST_F(NodeSchedulerTest, LinkedNodesGetsUpdated) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto expected_map = reuse_map_;

  std::vector<std::pair<std::string, int>> target = {
      {query_node_b_->node_name, 0}};
  std::map<int, std::vector<std::pair<std::string, int>>> target_map;
  target_map.insert({0, target});

  expected_map.insert({query_node_a_->node_name, target_map});

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(reuse_map_, expected_map);
}

TEST_F(NodeSchedulerTest, LinkedNodesStaysSame) {
  query_node_a_->next_nodes = {query_node_b_};
  query_node_b_->previous_nodes = {query_node_a_};

  ModulesCombo expected_module_vector = {{query_node_a_->operation_type, {}},
                                         {query_node_b_->operation_type, {}}};

  base_supported_bitstreams_.insert({expected_module_vector, "Some_bitstream"});

  std::vector<std::shared_ptr<Node>> starting_nodes = {query_node_a_};

  auto expected_map = reuse_map_;

  auto scheduling_results = NodeScheduler::FindAcceleratedQueryNodeSets(
      starting_nodes, base_supported_bitstreams_, base_existing_modules_,
      reuse_map_);

  ASSERT_EQ(reuse_map_, expected_map);
}

}  // namespace