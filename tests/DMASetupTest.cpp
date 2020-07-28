#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdio>

#include "DMASetup.hpp"
#include "MockDMA.hpp"
namespace {
int input_stream_id = 0;
int output_stream_id = 1;

TEST(DMASetupTest, InputParamsSettings) {
  int expected_ddr_burst_length = 72;
  int expected_records_per_ddr_burst = 16;
  int expected_buffer_start = 0;
  int expected_buffer_end = 15;
  int expected_stream_size = 1000;
  std::vector<int> mock_db_data(1, 0);  // NOLINT
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, setInputControllerParams(
                            input_stream_id, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, setInputControllerStreamAddress(
                            input_stream_id,
                            reinterpret_cast<uintptr_t>(&mock_db_data[0])))
      .Times(1);
  EXPECT_CALL(mock_dma, setInputControllerStreamSize(input_stream_id,
                                                     expected_stream_size))
      .Times(1);
  DMASetup dma_configurer;

  dma_configurer.SetupDMAModule(mock_dma, mock_db_data, 18,
                                expected_stream_size, input_stream_id,
                                output_stream_id);
}
TEST(DMASetupTest, OutputParamsSettings) {
  int expected_ddr_burst_length = 72;
  int expected_records_per_ddr_burst = 16;
  int expected_buffer_start = 0;
  int expected_buffer_end = 15;
  std::vector<int> mock_db_data(1, 0);  // NOLINT
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, setOutputControllerParams(
                            output_stream_id, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, setOutputControllerStreamAddress(
                            output_stream_id,
                            reinterpret_cast<uintptr_t>(&mock_db_data[0])))
      .Times(1);
  EXPECT_CALL(mock_dma, setOutputControllerStreamSize(output_stream_id, 0))
      .Times(1);
  DMASetup dma_configurer;
  dma_configurer.SetupDMAModule(mock_dma, mock_db_data, 18, 1000,
                                input_stream_id, output_stream_id);
}
TEST(DMASetupTest, RecordSettings) {
  std::vector<int> mock_db_data(1, 0);  // NOLINT
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, setRecordSize(output_stream_id, 2)).Times(1);
  EXPECT_CALL(mock_dma, setRecordSize(input_stream_id, 2)).Times(1);
  EXPECT_CALL(mock_dma, setRecordChunkIDs(output_stream_id, 0, 0)).Times(1);
  EXPECT_CALL(mock_dma, setRecordChunkIDs(output_stream_id, 1, 1)).Times(1);
  EXPECT_CALL(mock_dma, setRecordChunkIDs(input_stream_id, 0, 0)).Times(1);
  EXPECT_CALL(mock_dma, setRecordChunkIDs(input_stream_id, 1, 1)).Times(1);
  DMASetup dma_configurer;
  dma_configurer.SetupDMAModule(mock_dma, mock_db_data, 18, 1000,
                                input_stream_id, output_stream_id);
}
}  // namespace