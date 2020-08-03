#include "dma_setup.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_dma.hpp"
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
  int* mock_output_memory_address = static_cast<int*>(malloc(1));
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, SetInputControllerParams(
                            input_stream_id, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, SetInputControllerStreamAddress(
                            input_stream_id,
                            reinterpret_cast<uintptr_t>(&mock_db_data[0])))
      .Times(1);
  EXPECT_CALL(mock_dma, SetInputControllerStreamSize(input_stream_id,
                                                     expected_stream_size))
      .Times(1);
  DMASetup dma_configurer;

  dma_configurer.SetupDMAModule(
      mock_dma, mock_db_data, mock_output_memory_address, 18,
      expected_stream_size, input_stream_id, output_stream_id);
  free(mock_output_memory_address);
}
TEST(DMASetupTest, OutputParamsSettings) {
  int expected_ddr_burst_length = 72;
  int expected_records_per_ddr_burst = 16;
  int expected_buffer_start = 0;
  int expected_buffer_end = 15;
  std::vector<int> mock_db_data(1, 0);  // NOLINT
  int* mock_output_memory_address = static_cast<int*>(malloc(1));
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, SetOutputControllerParams(
                            output_stream_id, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, SetOutputControllerStreamAddress(
                            output_stream_id, reinterpret_cast<uintptr_t>(
                                                  mock_output_memory_address)))
      .Times(1);
  EXPECT_CALL(mock_dma, SetOutputControllerStreamSize(output_stream_id, 0))
      .Times(1);
  DMASetup dma_configurer;
  dma_configurer.SetupDMAModule(mock_dma, mock_db_data,
                                mock_output_memory_address, 18, 1000,
                                input_stream_id, output_stream_id);
  free(mock_output_memory_address);
}
TEST(DMASetupTest, RecordSettings) {
  std::vector<int> mock_db_data(1, 0);  // NOLINT
  int* mock_output_memory_address = static_cast<int*>(malloc(1));
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, SetRecordSize(output_stream_id, 2)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordSize(input_stream_id, 2)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(output_stream_id, 0, 0)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(output_stream_id, 1, 1)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(input_stream_id, 0, 0)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(input_stream_id, 1, 1)).Times(1);
  DMASetup dma_configurer;
  dma_configurer.SetupDMAModule(mock_dma, mock_db_data,
                                mock_output_memory_address, 18, 1000,
                                input_stream_id, output_stream_id);
}
}  // namespace