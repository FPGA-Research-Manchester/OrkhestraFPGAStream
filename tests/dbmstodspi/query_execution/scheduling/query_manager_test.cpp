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

#include "query_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>

#include "mock_accelerator_library.hpp"
#include "mock_data_manager.hpp"
#include "mock_fpga_manager.hpp"
#include "mock_memory_manager.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_data_parameters.hpp"
#include "virtual_memory_block.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::
    NodeOperationParameters;
using orkhestrafs::dbmstodspi::QueryManager;
using orkhestrafs::dbmstodspi::StreamDataParameters;
using orkhestrafs::dbmstodspi::VirtualMemoryBlock;

namespace {

class QueryManagerTest : public ::testing::Test {
 protected:
  std::shared_ptr<QueryNode> base_query_node_;
  QueryOperationType base_operation_type_ = QueryOperationType::kJoin;
  NodeOperationParameters any_operation_params_ = {{}, {}, {}};
  std::string base_node_name_ = "base";
  std::vector<std::string> data_files_vector_;
  std::vector<std::shared_ptr<QueryNode>> next_nodes_;
  std::vector<std::weak_ptr<QueryNode>> previous_nodes_;
  std::vector<bool> is_checked_;

  void SetUp() override {
    base_query_node_ = std::make_shared<QueryNode>(
        data_files_vector_, data_files_vector_, base_operation_type_,
        next_nodes_, previous_nodes_, any_operation_params_, base_node_name_,
        is_checked_);
  }
};

// Two links. One link is about current run and the other one is for the second
// run.
TEST_F(QueryManagerTest, GetCurrentLinksReturnsLinks) {
  /*std::map<std::string, std::map<int, MemoryReuseTargets>> all_reuse_targets;
  std::map<std::string, std::map<int, MemoryReuseTargets>> expected_results;
  std::map<int, MemoryReuseTargets> any_reuse_target;
  std::map<int, MemoryReuseTargets> expected_reuse_target;
  std::string other_node_name = "other";
  expected_reuse_target.insert({0, {{other_node_name, 0}}});
  auto other_query_node = std::make_shared<QueryNode>(
      data_files_vector_, data_files_vector_, base_operation_type_, next_nodes_,
      previous_nodes_, any_operation_params_, other_node_name, is_checked_);
  all_reuse_targets.insert({base_node_name_, expected_reuse_target});
  all_reuse_targets.insert({other_node_name, any_reuse_target});
  expected_results.insert({base_node_name_, expected_reuse_target});

  QueryManager query_manager_under_test(nullptr);

  ASSERT_EQ(expected_results, query_manager_under_test.GetCurrentLinks(all_reuse_targets));*/
}
// This method should be made smaller and unit tests more fine grain.
TEST_F(QueryManagerTest, SetupAccelerationNodesForExecutionReturnsExpectedRun) {
  //std::unique_ptr<MemoryBlockInterface> output_memory_block =
  //    std::make_unique<VirtualMemoryBlock>();
  //std::unique_ptr<MemoryBlockInterface> input_memory_block =
  //    std::make_unique<VirtualMemoryBlock>();

  //auto *input_physical_address = input_memory_block->GetPhysicalAddress();
  //auto *output_physical_address = output_memory_block->GetPhysicalAddress();

  //std::string input_filename = "input_file";
  //std::string output_filename = "output_file";

  //std::vector<std::pair<ColumnDataType, int>> expected_column_defs_vector = {
  //    {ColumnDataType::kDecimal, 2},
  //    {ColumnDataType::kDate, 4},
  //    {ColumnDataType::kDecimal, 1}};

  //std::vector<int> column_types = {3, 4, 3};
  //std::vector<ColumnDataType> expected_column_types = {
  //    ColumnDataType::kDecimal, ColumnDataType::kDate,
  //    ColumnDataType::kDecimal};
  //std::vector<int> column_widths = {4, 2, 2};
  //std::vector<int> input_projection = {1, 2, 3, 4, 5, 6, 7};
  //std::vector<int> output_projection = {2, 1, 3, 4, 5, 6, 7};
  //std::vector<int> empty_vector = {};
  //std::vector<int> chunk_count = {1};
  //std::vector<std::vector<int>> input_stream_specification = {
  //    input_projection, column_types, column_widths, empty_vector};
  //std::vector<std::vector<int>> output_stream_specification = {
  //    output_projection, column_types, column_widths, chunk_count};
  //std::vector<std::vector<int>> empty_operation_params;
  //int expected_record_count = 999;

  //MockDataManager mock_data_manager;
  //EXPECT_CALL(
  //    mock_data_manager,
  //    MockWriteDataFromCSVToMemory(input_filename, expected_column_defs_vector,
  //                                 input_memory_block.get()))
  //    .WillOnce(testing::Return(expected_record_count));
  //EXPECT_CALL(mock_data_manager,
  //            GetHeaderColumnVector(expected_column_types, column_widths))
  //    .WillOnce(testing::Return(expected_column_defs_vector))
  //    .WillOnce(testing::Return(expected_column_defs_vector));

  //MockMemoryManager mock_memory_manager;
  //EXPECT_CALL(mock_memory_manager, GetAvailableMemoryBlock())
  //    .WillOnce(
  //        testing::Return(testing::ByMove(std::move(output_memory_block))))
  //    .WillOnce(
  //        testing::Return(testing::ByMove(std::move(input_memory_block))));

  //MockAcceleratorLibrary mock_accelerator_library;
  //std::pair<int, int> expected_multi_channel_params = {-1, -1};
  //EXPECT_CALL(mock_accelerator_library,
  //            GetMultiChannelParams(true, 0, base_operation_type_,
  //                                  empty_operation_params))
  //    .WillOnce(testing::Return(expected_multi_channel_params));
  //EXPECT_CALL(mock_accelerator_library,
  //            GetMultiChannelParams(false, 0, base_operation_type_,
  //                                  empty_operation_params))
  //    .WillOnce(testing::Return(expected_multi_channel_params));

  //std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
  //    input_memory_blocks;
  //std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
  //    output_memory_blocks;
  //std::map<std::string, std::vector<RecordSizeAndCount>> input_stream_sizes;
  //std::map<std::string, std::vector<RecordSizeAndCount>> output_stream_sizes;

  //base_query_node_->input_data_definition_files.push_back(input_filename);
  //base_query_node_->output_data_definition_files.push_back(output_filename);
  //base_query_node_->next_nodes.push_back(nullptr);
  //base_query_node_->previous_nodes.push_back(std::weak_ptr<QueryNode>());
  //base_query_node_->operation_parameters.input_stream_parameters =
  //    input_stream_specification;
  //base_query_node_->operation_parameters.output_stream_parameters =
  //    output_stream_specification;

  //bool check_status = false;
  //base_query_node_->is_checked = {check_status};
  //int expected_location = 99;
  //// TODO: Missing test with composed modules!
  //base_query_node_->module_locations = {expected_location};

  //QueryManager query_manager_under_test(nullptr);

  //auto [query_nodes, result_parameters] =
  //    query_manager_under_test.SetupAccelerationNodesForExecution(
  //        &mock_data_manager, &mock_memory_manager, &mock_accelerator_library,
  //        input_memory_blocks, output_memory_blocks, input_stream_sizes,
  //        output_stream_sizes, {base_query_node_});

  //int expected_stream_id = 0;
  //int expected_record_size = 7;

  //StreamDataParameters expected_input_stream_param = {
  //    expected_stream_id, expected_record_size, expected_record_count,
  //    input_physical_address, input_projection};
  //StreamDataParameters expected_output_stream_param = {
  //    expected_stream_id,      expected_record_size, 0,
  //    output_physical_address, output_projection,    chunk_count.at(0)};

  //AcceleratedQueryNode expected_query_node = {{expected_input_stream_param},
  //                                            {expected_output_stream_param},
  //                                            base_operation_type_,
  //                                            expected_location,
  //                                            {},
  //                                            empty_operation_params};

  //int expected_stream_index = 0;

  //StreamResultParameters expected_result_params = {
  //    expected_stream_index, expected_stream_id, output_filename, check_status,
  //    output_stream_specification};

  //ASSERT_EQ(1, result_parameters.size());
  //ASSERT_EQ(1, result_parameters.at(base_node_name_).size());
  //ASSERT_EQ(1, query_nodes.size());
  //ASSERT_EQ(expected_query_node, query_nodes.at(0));
  //ASSERT_EQ(expected_result_params,
  //          result_parameters.at(base_node_name_).at(0));

  //ASSERT_EQ(1, input_stream_sizes.at(base_node_name_).size());
  //ASSERT_EQ(expected_record_size,
  //          input_stream_sizes.at(base_node_name_).at(0).first);
  //ASSERT_EQ(expected_record_count,
  //          input_stream_sizes.at(base_node_name_).at(0).second);
  //ASSERT_EQ(1, output_stream_sizes.at(base_node_name_).size());
  //ASSERT_EQ(expected_record_size,
  //          output_stream_sizes.at(base_node_name_).at(0).first);
  //ASSERT_EQ(0, output_stream_sizes.at(base_node_name_).at(0).second);

  //ASSERT_EQ(1, input_memory_blocks.at(base_node_name_).size());
  //ASSERT_EQ(
  //    input_physical_address,
  //    input_memory_blocks.at(base_node_name_).at(0)->GetPhysicalAddress());
  //ASSERT_EQ(1, output_memory_blocks.at(base_node_name_).size());
  //ASSERT_EQ(
  //    output_physical_address,
  //    output_memory_blocks.at(base_node_name_).at(0)->GetPhysicalAddress());
}
TEST_F(QueryManagerTest, LoadNextBitstreamIfNewUsesMemoryManager) {
  MockMemoryManager mock_memory_manager;
  QueryManager query_manager_under_test(nullptr);
  Config config;
  std::string expected_bitstream_file_name = "test_file";
  int expected_memory_space = 10;
  config.required_memory_space.insert(
      {expected_bitstream_file_name, expected_memory_space});

  EXPECT_CALL(
      mock_memory_manager,
      LoadBitstreamIfNew(expected_bitstream_file_name, expected_memory_space))
      .Times(1);

  query_manager_under_test.LoadNextBitstreamIfNew(
      &mock_memory_manager, expected_bitstream_file_name, config);
}
TEST_F(QueryManagerTest, DISABLED_ExecuteAndProcessResultsCallsFPGAManager) {
  MockFPGAManager mock_fpga_manager;

  std::vector<AcceleratedQueryNode> execution_query_nodes;

  EXPECT_CALL(mock_fpga_manager, SetupQueryAcceleration(execution_query_nodes))
      .Times(1);
  EXPECT_CALL(mock_fpga_manager, RunQueryAcceleration()).Times(1);

  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      output_memory_blocks;
  std::map<std::string, std::vector<RecordSizeAndCount>> output_stream_sizes;
  std::map<std::string, std::vector<StreamResultParameters>> result_parameters;
  MockDataManager mock_data_manager;
  QueryManager query_manager_under_test(nullptr);
  /*query_manager_under_test.ExecuteAndProcessResults(
      &mock_fpga_manager, &mock_data_manager, output_memory_blocks,
      output_stream_sizes, result_parameters, execution_query_nodes);*/
}

TEST_F(QueryManagerTest, FreeMemoryBlocksMovesLinkedMemoryBlocks) {
  QueryManager query_manager_under_test(nullptr);
  MockMemoryManager mock_memory_manager;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      input_memory_blocks;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      output_memory_blocks;
  std::map<std::string, std::vector<RecordSizeAndCount>> input_stream_sizes;
  std::map<std::string, std::vector<RecordSizeAndCount>> output_stream_sizes;
  std::map<std::string, std::map<int, MemoryReuseTargets>> reuse_links;
  std::vector<std::string> scheduled_node_names;

  std::unique_ptr<MemoryBlockInterface> expected_memory_block =
      std::make_unique<VirtualMemoryBlock>();
  auto *expected_physical_address = expected_memory_block->GetPhysicalAddress();
  std::string target_node_name = "target";
  std::string source_node_name = "source";
  std::vector<std::unique_ptr<MemoryBlockInterface>> source_input_vector;
  source_input_vector.push_back(nullptr);
  std::vector<std::unique_ptr<MemoryBlockInterface>> target_input_vector;
  target_input_vector.push_back(nullptr);
  input_memory_blocks.insert(
      {target_node_name, std::move(target_input_vector)});
  input_memory_blocks.insert(
      {source_node_name, std::move(source_input_vector)});
  std::vector<std::unique_ptr<MemoryBlockInterface>> source_vector;
  source_vector.push_back(std::move(expected_memory_block));
  output_memory_blocks.insert({source_node_name, std::move(source_vector)});
  RecordSizeAndCount expected_record_data = {11, 22};
  RecordSizeAndCount other_record_data = {0, 0};
  input_stream_sizes.insert({target_node_name, {other_record_data}});
  input_stream_sizes.insert({source_node_name, {other_record_data}});
  output_stream_sizes.insert({source_node_name, {expected_record_data}});

  std::map<int, MemoryReuseTargets> target_reuse_link;
  target_reuse_link.insert({0, {{target_node_name, 0}}});
  reuse_links.insert({source_node_name, target_reuse_link});

  scheduled_node_names.push_back(source_node_name);

  query_manager_under_test.FreeMemoryBlocks(
      &mock_memory_manager, input_memory_blocks, output_memory_blocks,
      input_stream_sizes, output_stream_sizes, reuse_links,
      scheduled_node_names);

  ASSERT_EQ(1, input_memory_blocks.size());
  ASSERT_EQ(1, input_memory_blocks.at(target_node_name).size());
  ASSERT_EQ(
      expected_physical_address,
      input_memory_blocks.at(target_node_name).at(0)->GetPhysicalAddress());
  ASSERT_EQ(0, output_memory_blocks.size());
  ASSERT_EQ(1, input_stream_sizes.size());
  ASSERT_EQ(1, input_stream_sizes.at(target_node_name).size());
  ASSERT_EQ(expected_record_data.first,
            input_stream_sizes.at(target_node_name).at(0).first);
  ASSERT_EQ(expected_record_data.second,
            input_stream_sizes.at(target_node_name).at(0).second);
  ASSERT_EQ(0, output_stream_sizes.size());
}

TEST_F(QueryManagerTest, FreeMemoryBlocksRemovesMemoryBlocksAndStreamData) {
  QueryManager query_manager_under_test(nullptr);
  MockMemoryManager mock_memory_manager;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      input_memory_blocks;
  std::map<std::string, std::vector<std::unique_ptr<MemoryBlockInterface>>>
      output_memory_blocks;
  std::map<std::string, std::vector<RecordSizeAndCount>> input_stream_sizes;
  std::map<std::string, std::vector<RecordSizeAndCount>> output_stream_sizes;
  std::map<std::string, std::map<int, MemoryReuseTargets>> reuse_links;
  std::vector<std::string> scheduled_node_names;

  std::string node_name = "node";
  std::unique_ptr<MemoryBlockInterface> expected_output_memory_block =
      std::make_unique<VirtualMemoryBlock>();
  std::vector<std::unique_ptr<MemoryBlockInterface>> output_vector;
  output_vector.push_back(std::move(expected_output_memory_block));
  output_memory_blocks.insert({node_name, std::move(output_vector)});

  std::unique_ptr<MemoryBlockInterface> expected_input_memory_block =
      std::make_unique<VirtualMemoryBlock>();
  std::vector<std::unique_ptr<MemoryBlockInterface>> input_vector;
  input_vector.push_back(std::move(expected_input_memory_block));
  input_memory_blocks.insert({node_name, std::move(input_vector)});

  RecordSizeAndCount record_data = {11, 22};
  input_stream_sizes.insert({node_name, {record_data}});
  output_stream_sizes.insert({node_name, {record_data}});

  scheduled_node_names.push_back(node_name);

  query_manager_under_test.FreeMemoryBlocks(
      &mock_memory_manager, input_memory_blocks, output_memory_blocks,
      input_stream_sizes, output_stream_sizes, reuse_links,
      scheduled_node_names);

  ASSERT_EQ(0, input_memory_blocks.size());
  ASSERT_EQ(0, output_memory_blocks.size());
  ASSERT_EQ(0, input_stream_sizes.size());
  ASSERT_EQ(0, output_stream_sizes.size());
}

}  // namespace