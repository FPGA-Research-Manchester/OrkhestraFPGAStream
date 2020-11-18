#include "dma_setup.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_dma.hpp"
namespace {
const int kInputStreamId = 0;
const int kOutputStreamId = 0;

TEST(DMASetupTest, InputParamsSettings) {
  int expected_ddr_burst_length = 72;
  int expected_records_per_ddr_burst = 16;
  int expected_buffer_start = 0;
  int expected_buffer_end = 15;
  int expected_stream_size = 1000;
  std::vector<uint32_t> mock_db_data(1, 0);  // NOLINT
  auto* mock_output_memory_address = static_cast<uint32_t*>(malloc(1));
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, SetInputControllerParams(
                            kInputStreamId, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, SetInputControllerStreamAddress(
                            kInputStreamId,
                            reinterpret_cast<uintptr_t>(&mock_db_data[0])))
      .Times(1);
  EXPECT_CALL(mock_dma, SetInputControllerStreamSize(kInputStreamId,
                                                     expected_stream_size))
      .Times(1);

  std::vector<StreamInitialisationData> input_streams = {
      {kInputStreamId, 18, expected_stream_size, mock_db_data.data()}};
  std::vector<StreamInitialisationData> output_streams = {
      {kOutputStreamId, 18, 0, mock_output_memory_address}};

  DMASetup dma_configurer;
  dma_configurer.SetupDMAModule(mock_dma, input_streams, output_streams);
  free(mock_output_memory_address);
}
TEST(DMASetupTest, OutputParamsSettings) {
  int expected_ddr_burst_length = 72;
  int expected_records_per_ddr_burst = 16;
  int expected_buffer_start = 0;
  int expected_buffer_end = 15;
  std::vector<uint32_t> mock_db_data(1, 0);  // NOLINT
  auto* mock_output_memory_address = static_cast<uint32_t*>(malloc(1));
  MockDMA mock_dma;
  EXPECT_CALL(mock_dma, SetOutputControllerParams(
                            kOutputStreamId, expected_ddr_burst_length,
                            expected_records_per_ddr_burst,
                            expected_buffer_start, expected_buffer_end))
      .Times(1);
  EXPECT_CALL(mock_dma, SetOutputControllerStreamAddress(
                            kOutputStreamId, reinterpret_cast<uintptr_t>(
                                                 mock_output_memory_address)))
      .Times(1);
  EXPECT_CALL(mock_dma, SetOutputControllerStreamSize(kOutputStreamId, 0))
      .Times(1);

  std::vector<StreamInitialisationData> input_streams = {
      {kInputStreamId, 18, 1000, mock_db_data.data()}};
  std::vector<StreamInitialisationData> output_streams = {
      {kOutputStreamId, 18, 0, mock_output_memory_address}};

  DMASetup dma_configurer;
  dma_configurer.SetupDMAModule(mock_dma, input_streams, output_streams);
  free(mock_output_memory_address);
}
TEST(DMASetupTest, RecordSettings) {
  std::vector<uint32_t> mock_db_data(1, 0);  // NOLINT
  auto* mock_output_memory_address = static_cast<uint32_t*>(malloc(1));
  MockDMA mock_dma;
  // Output stream configuration checks are possibly not needed.
  /*EXPECT_CALL(mock_dma, SetRecordSize(kOutputStreamId, 2)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(kOutputStreamId, 0, 0)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(kOutputStreamId, 1, 1)).Times(1);*/
  EXPECT_CALL(mock_dma, SetRecordSize(kInputStreamId, 2)).Times(1);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(kInputStreamId, testing::_, 0))
      .Times(16);
  EXPECT_CALL(mock_dma, SetRecordChunkIDs(kInputStreamId, testing::_, 1))
      .Times(16);

    std::vector<StreamInitialisationData> input_streams = {
      {kInputStreamId, 18, 1000, mock_db_data.data()}};
  std::vector<StreamInitialisationData> output_streams = {
      {kOutputStreamId, 18, 0, mock_output_memory_address}};

  DMASetup dma_configurer;
  dma_configurer.SetupDMAModule(mock_dma, input_streams, output_streams);
  free(mock_output_memory_address);
}
}  // namespace