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

#include "table_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_data_manager.hpp"
#include "virtual_memory_block.hpp"

using orkhestrafs::dbmstodspi::TableManager;
using orkhestrafs::dbmstodspi::VirtualMemoryBlock;

namespace {

class TableManagerTest : public ::testing::Test {
 protected:
  TableData test_table_;
  std::vector<std::pair<ColumnDataType, int>> test_column_defs_vector_ = {
      {ColumnDataType::kDecimal, 2},
      {ColumnDataType::kDate, 4},
      {ColumnDataType::kDecimal, 1}};
  std::vector<uint32_t> test_data_vector_ = {0, 1, 2, 3, 4, 5, 6};

  void SetUp() override {
    test_table_.table_column_label_vector = test_column_defs_vector_;
    test_table_.table_data_vector = test_data_vector_;
  }

  MockDataManager mock_data_manager_;
  std::vector<int> any_vector_;
  std::string any_filename_;
  std::unique_ptr<MemoryBlockInterface> memory_device_ =
      std::make_unique<VirtualMemoryBlock>();
};
TEST_F(TableManagerTest, RecordSizeFromTableIsCorrect) {
  ASSERT_EQ(7, TableManager::GetRecordSizeFromTable(test_table_));
}

TEST_F(TableManagerTest, GetColumnDefsVectorReturnsExpectedVector) {
  std::vector<std::pair<ColumnDataType, int>> expected_column_defs_vector = {
      {ColumnDataType::kDecimal, 2},
      {ColumnDataType::kDate, 4},
      {ColumnDataType::kDecimal, 1}};

  std::vector<int> column_types = {3, 4, 3};
  std::vector<ColumnDataType> expected_column_types = {
      ColumnDataType::kDecimal, ColumnDataType::kDate,
      ColumnDataType::kDecimal};
  std::vector<int> column_widths = {4, 2, 2};
  int stream_index = 1;
  std::vector<std::vector<int>> stream_specification = {
      any_vector_, any_vector_,  any_vector_,   any_vector_,
      any_vector_, column_types, column_widths, any_vector_};

  EXPECT_CALL(mock_data_manager_,
              GetHeaderColumnVector(expected_column_types, column_widths))
      .WillOnce(testing::Return(expected_column_defs_vector));

  auto resuling_column_defs = TableManager::GetColumnDefsVector(
      &mock_data_manager_, stream_specification, stream_index);

  ASSERT_EQ(expected_column_defs_vector.at(0), resuling_column_defs.at(0));
  ASSERT_EQ(expected_column_defs_vector.at(1), resuling_column_defs.at(1));
  ASSERT_EQ(expected_column_defs_vector.at(2), resuling_column_defs.at(2));
}

TEST_F(TableManagerTest, WriteDataToMemoryReturnsCorrectRecordData) {
  std::vector<std::pair<ColumnDataType, int>> expected_column_defs_vector;
  int stream_index = 0;
  std::vector<std::vector<int>> stream_specification = {{}, {}, {}, {}};

  int expected_record_count = 123;
  std::pair<int, int> expected_record_data = {0, expected_record_count};

  EXPECT_CALL(
      mock_data_manager_,
      MockWriteDataFromCSVToMemory(any_filename_, expected_column_defs_vector,
                                   memory_device_.get()))
      .WillOnce(testing::Return(expected_record_count));

  ASSERT_EQ(expected_record_data,
            TableManager::WriteDataToMemory(&mock_data_manager_,
                                            stream_specification, stream_index,
                                            memory_device_, any_filename_));
}

}  // namespace