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

#include "data_manager.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_csv_reader.hpp"
#include "virtual_memory_block.hpp"
namespace {

using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::dbmstodspi::DataManager;
using orkhestrafs::dbmstodspi::VirtualMemoryBlock;

class DataManagerTest : public ::testing::Test {
 protected:
  DataManagerTest()
      : csv_reader_ptr_(std::make_unique<MockCSVReader>()),
        mock_csv_reader_(*csv_reader_ptr_) {}

  std::unique_ptr<MockCSVReader> csv_reader_ptr_;
  char any_separator_ = ',';
  std::map<ColumnDataType, double> any_data_sizes_ = {
      {ColumnDataType::kDate, 2}, {ColumnDataType::kDecimal, 0.5}};
  MockCSVReader& mock_csv_reader_;
};

TEST_F(DataManagerTest, WriteDataFromCSVToMemoryUsesReader) {
  int expected_return = 10;
  std::string test_filename = "test";
  std::vector<std::pair<ColumnDataType, int>> column_defs_vector;
  std::unique_ptr<MemoryBlockInterface> memory_device =
      std::make_unique<VirtualMemoryBlock>();

  EXPECT_CALL(mock_csv_reader_, WriteTableFromFileToMemory(
                                    test_filename, any_separator_,
                                    column_defs_vector, memory_device.get()))
      .WillOnce(testing::Return(expected_return));
  DataManager data_manager_under_test(any_data_sizes_, any_separator_,
                                      std::move(csv_reader_ptr_));
  ASSERT_EQ(expected_return,
            data_manager_under_test.WriteDataFromCSVToMemory(
                test_filename, column_defs_vector, memory_device.get()));
}

TEST_F(DataManagerTest, GetHeaderColumnVectorReturnsModifiedSizes) {
  std::vector<std::pair<ColumnDataType, int>> expected_column_defs_vector = {
      {ColumnDataType::kDecimal, 2},
      {ColumnDataType::kDate, 4},
      {ColumnDataType::kDecimal, 1}};

  std::vector<ColumnDataType> column_types = {ColumnDataType::kDecimal,
                                              ColumnDataType::kDate,
                                              ColumnDataType::kDecimal};
  std::vector<int> column_widths = {4, 2, 2};

  DataManager data_manager_under_test(any_data_sizes_, any_separator_,
                                      std::move(csv_reader_ptr_));

  ASSERT_EQ(expected_column_defs_vector,
            data_manager_under_test.GetHeaderColumnVector(column_types,
                                                          column_widths));
}

}  // namespace