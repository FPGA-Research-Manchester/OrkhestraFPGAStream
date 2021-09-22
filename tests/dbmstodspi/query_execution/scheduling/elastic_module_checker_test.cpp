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

#include "elastic_module_checker.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

using orkhestrafs::dbmstodspi::ElasticModuleChecker;
using orkhestrafs::dbmstodspi::StreamDataParameters;

class ElasticModuleCheckerTest : public ::testing::Test {
 protected:
  std::vector<StreamDataParameters> input_stream_parameters_;
  std::vector<std::vector<int>> operation_parameters_;
  const int any_stream_record_size_ = 1;
  uint32_t* any_address_pointer_ = nullptr;
  std::vector<int> any_stream_specification_;
};

TEST_F(ElasticModuleCheckerTest, MergeSortIsValid) {
  int max_channel_count = 1;
  int sorted_sequences_count = 1;
  int stream_record_count_to_sort = 1;
  int stream_id = 0;

  StreamDataParameters stream_parameters = {
      stream_id, any_stream_record_size_,
      stream_record_count_to_sort,
      any_address_pointer_, any_stream_specification_};

  input_stream_parameters_.push_back(stream_parameters);
  operation_parameters_.push_back({max_channel_count, sorted_sequences_count});

  ASSERT_TRUE(ElasticModuleChecker::IsRunValid(input_stream_parameters_,
                                               QueryOperationType::kMergeSort,
                                               operation_parameters_));
}

TEST_F(ElasticModuleCheckerTest, MergeSortHasTooManyStreams) {
  int max_channel_count = 1;
  int sorted_sequences_count = 1;
  int stream_record_count_to_sort = 1;
  int stream_id = 0;

  StreamDataParameters stream_parameters = {
      stream_id, any_stream_record_size_, stream_record_count_to_sort,
      any_address_pointer_, any_stream_specification_};

  input_stream_parameters_.push_back(stream_parameters);
  input_stream_parameters_.push_back(stream_parameters);
  operation_parameters_.push_back({max_channel_count, sorted_sequences_count});

  ASSERT_FALSE(ElasticModuleChecker::IsRunValid(input_stream_parameters_,
                                               QueryOperationType::kMergeSort,
                                               operation_parameters_));

  input_stream_parameters_.clear();
  input_stream_parameters_.push_back(stream_parameters);
  operation_parameters_.push_back({max_channel_count, sorted_sequences_count});
  ASSERT_FALSE(ElasticModuleChecker::IsRunValid(input_stream_parameters_,
                                                QueryOperationType::kMergeSort,
                                                operation_parameters_));
}

TEST_F(ElasticModuleCheckerTest, MergeSortHasIncorrectOperationParams) {
  int max_channel_count = 1;
  int sorted_sequences_count = 1;
  int stream_record_count_to_sort = 1;
  int stream_id = 0;
  int random_param = 0;

  StreamDataParameters stream_parameters = {
      stream_id, any_stream_record_size_, stream_record_count_to_sort,
      any_address_pointer_, any_stream_specification_};

  input_stream_parameters_.push_back(stream_parameters);
  operation_parameters_.push_back(
      {max_channel_count, sorted_sequences_count, random_param});

  ASSERT_FALSE(ElasticModuleChecker::IsRunValid(input_stream_parameters_,
                                               QueryOperationType::kMergeSort,
                                               operation_parameters_));

  operation_parameters_.clear();
  operation_parameters_.emplace_back();

  ASSERT_FALSE(ElasticModuleChecker::IsRunValid(input_stream_parameters_,
                                               QueryOperationType::kMergeSort,
                                               operation_parameters_));
}

TEST_F(ElasticModuleCheckerTest, MergeSorterIsNotBigEnough) {
  int max_channel_count = 1;
  int sorted_sequences_count = 1;
  int stream_record_count_to_sort = 2;
  int stream_id = 0;

  StreamDataParameters stream_parameters = {
      stream_id, any_stream_record_size_, stream_record_count_to_sort,
      any_address_pointer_, any_stream_specification_};

  input_stream_parameters_.push_back(stream_parameters);
  operation_parameters_.push_back({max_channel_count, sorted_sequences_count});

  ASSERT_FALSE(ElasticModuleChecker::IsRunValid(input_stream_parameters_,
                                               QueryOperationType::kMergeSort,
                                               operation_parameters_));
}

TEST_F(ElasticModuleCheckerTest, FilterPassesMergeSortCheck) {
  int stream_record_count = 100;
  int stream_id = 0;

  StreamDataParameters stream_parameters = {
      stream_id, any_stream_record_size_, stream_record_count,
      any_address_pointer_, any_stream_specification_};

  input_stream_parameters_.push_back(stream_parameters);
  operation_parameters_.emplace_back();

  ASSERT_TRUE(ElasticModuleChecker::IsRunValid(input_stream_parameters_,
                                               QueryOperationType::kFilter,
                                               operation_parameters_));
}
}  // namespace