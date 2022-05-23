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

#include "dma_setup.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_dma.hpp"
namespace {
using orkhestrafs::dbmstodspi::DMASetup;
using orkhestrafs::dbmstodspi::StreamDataParameters;

const int kInputStreamId = 0;
const int kOutputStreamId = 0;

TEST(DMASetupTest, SetupDMAModuleSetsCorrectParams) {
  int expected_ddr_burst_length = 72;
  int expected_records_per_ddr_burst = 16;
  int expected_buffer_start = 0;
  int expected_buffer_end = 15;
  int expected_stream_size = 1000;
  std::vector<uint32_t> mock_db_data(1, 0);  // NOLINT
  auto* mock_output_memory_address = static_cast<uint32_t*>(malloc(1));
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, SetControllerParams(
                            true, kInputStreamId, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, SetControllerParams(
                            false, kOutputStreamId, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, SetControllerStreamAddress(
                            true, kInputStreamId,
                            reinterpret_cast<uintptr_t>(&mock_db_data[0])))
      .Times(1);
  EXPECT_CALL(mock_dma,
              SetControllerStreamAddress(
                  false, kOutputStreamId,
                  reinterpret_cast<uintptr_t>(mock_output_memory_address)))
      .Times(1);
  EXPECT_CALL(mock_dma, SetControllerStreamSize(true, kInputStreamId,
                                                expected_stream_size))
      .Times(1);
  EXPECT_CALL(mock_dma, SetControllerStreamSize(false, kOutputStreamId, 0))
      .Times(1);

  std::map<volatile uint32_t*, std::vector<int>> input_map = {
      {mock_db_data.data(), {}}};
  std::vector<StreamDataParameters> input_streams = {{kInputStreamId,
                                                      18,
                                                      expected_stream_size, input_map,
                                                      {}}};
  std::map<volatile uint32_t*, std::vector<int>> output_map = {
      {mock_output_memory_address, {}}};
  std::vector<StreamDataParameters> output_streams = {
      {kOutputStreamId, 18, 0, output_map, {}, 2}};

  DMASetup setup_under_test;
  setup_under_test.SetupDMAModule(mock_dma, input_streams, output_streams);
  free(mock_output_memory_address);
}
TEST(DMASetupTest, RecordSettings) {
  std::vector<uint32_t> mock_db_data(1, 0);  // NOLINT
  auto* mock_output_memory_address = static_cast<uint32_t*>(malloc(1));
  MockDMA mock_dma;
  // Output stream configuration checks are possibly not needed.
  // EXPECT_CALL(mock_dma, SetRecordSize(kOutputStreamId, 2)).Times(1);
  // EXPECT_CALL(mock_dma, SetRecordChunkIDs(kOutputStreamId, 0, 0)).Times(1);
  // EXPECT_CALL(mock_dma, SetRecordChunkIDs(kOutputStreamId, 1, 1)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordSize(kInputStreamId, 2)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(kInputStreamId, testing::_, 0))
      .Times(16);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(kInputStreamId, testing::_, 1))
      .Times(16);
  std::map<volatile uint32_t*, std::vector<int>> input_map = {
      {mock_db_data.data(), {}}};
  std::map<volatile uint32_t*, std::vector<int>> output_map = {
      {mock_output_memory_address, {}}};
  std::vector<StreamDataParameters> input_streams = {
      {kInputStreamId, 18, 1000, input_map, {}}};
  std::vector<StreamDataParameters> output_streams = {
      {kOutputStreamId, 18, 0, output_map, {}, 2}};

  DMASetup setup_under_test;
  setup_under_test.SetupDMAModule(mock_dma, input_streams, output_streams);
  free(mock_output_memory_address);
}
}  // namespace