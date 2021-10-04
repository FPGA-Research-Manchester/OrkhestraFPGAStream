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

#include "id_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
namespace {

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::query_scheduling_data::
    NodeOperationParameters;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;
using orkhestrafs::dbmstodspi::IDManager;

class IDManagerTest : public ::testing::Test {
 protected:
  std::map<std::string, std::vector<int>> input_ids_, output_ids_,
      expected_input_ids_, expected_output_ids_;
  const QueryOperationType base_operation_type_ = QueryOperationType::kFilter;
  const NodeOperationParameters any_operation_parameters_;
  const std::vector<bool> any_is_checked_;

  const std::string first_node_name_ = "A";
  const std::string second_node_name_ = "B";
  const std::string third_node_name_ = "C";

  const std::vector<std::string> one_data_file_vector_ = {""};
  std::vector<std::weak_ptr<QueryNode>> empty_previous_nodes_vector_ = {};
  std::vector<std::shared_ptr<QueryNode>> empty_next_nodes_vector_ = {};
};

TEST_F(IDManagerTest, SingleNode1In1Out) {
  // Don't have to be nullptrs
  std::vector<std::weak_ptr<QueryNode>> previous_nodes = {
      std::weak_ptr<QueryNode>()};
  std::vector<std::shared_ptr<QueryNode>> next_nodes = {nullptr};

  QueryNode first_node(one_data_file_vector_, one_data_file_vector_,
                       base_operation_type_, next_nodes, previous_nodes,
                       any_operation_parameters_, first_node_name_,
                       any_is_checked_);

  expected_input_ids_.insert({first_node_name_, {0}});
  expected_output_ids_.insert({first_node_name_, {0}});

  IDManager::AllocateStreamIDs({first_node}, input_ids_, output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, SingleNodeNIn1Out) {
  std::vector<std::weak_ptr<QueryNode>> previous_nodes = {
      std::weak_ptr<QueryNode>(), std::weak_ptr<QueryNode>(),
      std::weak_ptr<QueryNode>()};
  std::vector<std::shared_ptr<QueryNode>> next_nodes = {nullptr};

  std::vector<std::string> input_data_files = {"", "", ""};
  std::vector<std::string> output_data_files = {""};

  QueryNode first_node(input_data_files, output_data_files,
                       base_operation_type_, next_nodes, previous_nodes,
                       any_operation_parameters_, first_node_name_,
                       any_is_checked_);

  expected_input_ids_.insert({first_node_name_, {0, 1, 2}});
  expected_output_ids_.insert({first_node_name_, {0}});

  IDManager::AllocateStreamIDs({first_node}, input_ids_, output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, SingleNode1InMOut) {
  std::vector<std::weak_ptr<QueryNode>> previous_nodes = {
      std::weak_ptr<QueryNode>()};
  std::vector<std::shared_ptr<QueryNode>> next_nodes = {nullptr, nullptr,
                                                        nullptr, nullptr};

  std::vector<std::string> input_data_files = {""};
  std::vector<std::string> output_data_files = {"", "", "", ""};

  QueryNode first_node(input_data_files, output_data_files,
                       base_operation_type_, next_nodes, previous_nodes,
                       any_operation_parameters_, first_node_name_,
                       any_is_checked_);

  expected_input_ids_.insert({first_node_name_, {0}});
  expected_output_ids_.insert({first_node_name_, {0, 1, 2, 3}});

  IDManager::AllocateStreamIDs({first_node}, input_ids_, output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, SingleNodeNInMOut) {
  std::vector<std::weak_ptr<QueryNode>> previous_nodes = {
      std::weak_ptr<QueryNode>(), std::weak_ptr<QueryNode>()};
  std::vector<std::shared_ptr<QueryNode>> next_nodes = {nullptr, nullptr,
                                                        nullptr};

  std::vector<std::string> input_data_files = {"", ""};
  std::vector<std::string> output_data_files = {"", "", ""};

  QueryNode first_node(input_data_files, output_data_files,
                       base_operation_type_, next_nodes, previous_nodes,
                       any_operation_parameters_, first_node_name_,
                       any_is_checked_);

  expected_input_ids_.insert({first_node_name_, {0, 1}});
  expected_output_ids_.insert({first_node_name_, {0, 1, 2}});

  IDManager::AllocateStreamIDs({first_node}, input_ids_, output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, TwoPipelinedNodes) {
  auto first_node = std::make_shared<QueryNode>(
      one_data_file_vector_, one_data_file_vector_, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, first_node_name_, any_is_checked_);
  auto second_node = std::make_shared<QueryNode>(
      one_data_file_vector_, one_data_file_vector_, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, second_node_name_, any_is_checked_);

  first_node->next_nodes.push_back(second_node);
  first_node->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  second_node->next_nodes.push_back(nullptr);
  second_node->previous_nodes.push_back(first_node);

  expected_input_ids_.insert({first_node_name_, {0}});
  expected_input_ids_.insert({second_node_name_, {0}});
  expected_output_ids_.insert({first_node_name_, {0}});
  expected_output_ids_.insert({second_node_name_, {0}});

  IDManager::AllocateStreamIDs({*first_node, *second_node}, input_ids_,
                               output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, TwoIndependentNodes) {
  std::vector<std::weak_ptr<QueryNode>> previous_nodes = {
      std::weak_ptr<QueryNode>()};
  std::vector<std::shared_ptr<QueryNode>> next_nodes = {nullptr};

  QueryNode first_node(one_data_file_vector_, one_data_file_vector_,
                       base_operation_type_, next_nodes, previous_nodes,
                       any_operation_parameters_, first_node_name_,
                       any_is_checked_);
  QueryNode second_node(one_data_file_vector_, one_data_file_vector_,
                        base_operation_type_, next_nodes, previous_nodes,
                        any_operation_parameters_, second_node_name_,
                        any_is_checked_);

  expected_input_ids_.insert({first_node_name_, {0}});
  expected_input_ids_.insert({second_node_name_, {1}});
  expected_output_ids_.insert({first_node_name_, {0}});
  expected_output_ids_.insert({second_node_name_, {1}});

  IDManager::AllocateStreamIDs({first_node, second_node}, input_ids_,
                               output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, TwoPipelinedNodesAndOneIndependent) {
  auto first_node = std::make_shared<QueryNode>(
      one_data_file_vector_, one_data_file_vector_, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, first_node_name_, any_is_checked_);
  auto second_node = std::make_shared<QueryNode>(
      one_data_file_vector_, one_data_file_vector_, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, second_node_name_, any_is_checked_);
  auto third_node = std::make_shared<QueryNode>(
      one_data_file_vector_, one_data_file_vector_, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, third_node_name_, any_is_checked_);

  first_node->next_nodes.push_back(second_node);
  first_node->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  second_node->next_nodes.push_back(nullptr);
  second_node->previous_nodes.push_back(first_node);
  third_node->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  third_node->next_nodes.push_back(nullptr);

  expected_input_ids_.insert({first_node_name_, {0}});
  expected_input_ids_.insert({second_node_name_, {0}});
  expected_input_ids_.insert({third_node_name_, {1}});
  expected_output_ids_.insert({first_node_name_, {0}});
  expected_output_ids_.insert({second_node_name_, {0}});
  expected_output_ids_.insert({third_node_name_, {1}});

  IDManager::AllocateStreamIDs({*first_node, *second_node, *third_node},
                               input_ids_, output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, TwoPipelineNodesWithDanglingOutputs) {
  std::vector<std::string> input_data_file_vector = {""};
  std::vector<std::string> output_data_file_vector = {"", ""};

  auto first_node = std::make_shared<QueryNode>(
      input_data_file_vector, output_data_file_vector, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, first_node_name_, any_is_checked_);
  auto second_node = std::make_shared<QueryNode>(
      input_data_file_vector, output_data_file_vector, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, second_node_name_, any_is_checked_);

  first_node->next_nodes.push_back(nullptr);
  first_node->next_nodes.push_back(second_node);
  first_node->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  second_node->next_nodes.push_back(nullptr);
  second_node->next_nodes.push_back(nullptr);
  second_node->previous_nodes.push_back(first_node);

  expected_input_ids_.insert({first_node_name_, {0}});
  expected_input_ids_.insert({second_node_name_, {1}});
  expected_output_ids_.insert({first_node_name_, {0, 1}});
  expected_output_ids_.insert({second_node_name_, {1, 2}});

  IDManager::AllocateStreamIDs({*first_node, *second_node}, input_ids_,
                               output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, TwoPipelinedNodesWithDanglingInputs) {
  std::vector<std::string> output_data_file_vector = {""};
  std::vector<std::string> input_data_file_vector = {"", ""};

  auto first_node = std::make_shared<QueryNode>(
      input_data_file_vector, output_data_file_vector, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, first_node_name_, any_is_checked_);
  auto second_node = std::make_shared<QueryNode>(
      input_data_file_vector, output_data_file_vector, base_operation_type_,
      empty_next_nodes_vector_, empty_previous_nodes_vector_,
      any_operation_parameters_, second_node_name_, any_is_checked_);

  first_node->next_nodes.push_back(second_node);
  first_node->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  first_node->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  second_node->next_nodes.push_back(nullptr);
  second_node->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  second_node->previous_nodes.push_back(first_node);

  expected_input_ids_.insert({first_node_name_, {0, 1}});
  expected_input_ids_.insert({second_node_name_, {2, 0}});
  expected_output_ids_.insert({first_node_name_, {0}});
  expected_output_ids_.insert({second_node_name_, {2}});

  IDManager::AllocateStreamIDs({*first_node, *second_node}, input_ids_,
                               output_ids_);
  ASSERT_EQ(expected_input_ids_, input_ids_);
  ASSERT_EQ(expected_output_ids_, output_ids_);
}

TEST_F(IDManagerTest, TooManyStreamsError) {
  std::vector<std::weak_ptr<QueryNode>> previous_nodes = {
      std::weak_ptr<QueryNode>()};
  std::vector<std::shared_ptr<QueryNode>> next_nodes = {nullptr};

  std::vector<QueryNode> all_nodes;

  for (int i = 0; i < 17; i++) {
    all_nodes.emplace_back(one_data_file_vector_, one_data_file_vector_,
                           base_operation_type_, next_nodes, previous_nodes,
                           any_operation_parameters_, std::to_string(i),
                           any_is_checked_);
  }

  ASSERT_THROW(IDManager::AllocateStreamIDs(all_nodes, input_ids_, output_ids_),
               std::runtime_error);
}

}  // namespace