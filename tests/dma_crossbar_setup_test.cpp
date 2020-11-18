#include "dma_crossbar_setup.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#include "dma_setup_data.hpp"
#include "query_acceleration_constants.hpp"
namespace {
const int kTestAnyChunk = 0;
const int kTestAnyPosition = 1;
const int kDatapathLength = query_acceleration_constants::kDatapathLength;
const int kDatapathWidth = query_acceleration_constants::kDatapathWidth;
const std::vector<int> kUntouchedVector(16, 0);

void GetGoldenConfigFromFile(std::vector<std::vector<int>>& golden_config,
                             const std::string& file_name) {
  std::ifstream input_file(file_name);
  ASSERT_TRUE(input_file);

  std::string line;
  while (std::getline(input_file, line)) {
    std::istringstream string_stream(line);
    int config_value = 0;
    std::vector<int> current_cycle_golden_config;
    while (string_stream >> config_value) {
      current_cycle_golden_config.push_back(config_value);
    }
    golden_config.push_back(current_cycle_golden_config);
  }
}

auto CalculateChunksPerRecord(int record_size) -> int {
  return (record_size + kDatapathWidth - 1) / kDatapathWidth;
}

void ExpectConfigurationDataIsUnconfigured(DMASetupData configuration_data) {
  for (int clock_cycle_index = 0; clock_cycle_index < kDatapathLength;
       clock_cycle_index++) {
    EXPECT_THAT(
        configuration_data.crossbar_setup_data[clock_cycle_index].chunk_data,
        testing::ElementsAreArray(kUntouchedVector));
    EXPECT_THAT(
        configuration_data.crossbar_setup_data[clock_cycle_index].position_data,
        testing::ElementsAreArray(kUntouchedVector));
  }
}

void ExpectConfigurationDataIsConfigured(
    DMASetupData configuration_data, std::string golden_chunk_data_file,
    std::string golden_position_data_file) {
  std::vector<std::vector<int>> golden_chunk_config;
  GetGoldenConfigFromFile(golden_chunk_config, golden_chunk_data_file);
  std::vector<std::vector<int>> golden_position_config;
  GetGoldenConfigFromFile(golden_position_config, golden_position_data_file);
  for (int clock_cycle_index = 0; clock_cycle_index < kDatapathLength;
       clock_cycle_index++) {
    EXPECT_THAT(
        configuration_data.crossbar_setup_data[clock_cycle_index].chunk_data,
        testing::ElementsAreArray(golden_chunk_config[clock_cycle_index]))
        << "Vectors differ at clock cycle: " << clock_cycle_index;
    EXPECT_THAT(
        configuration_data.crossbar_setup_data[clock_cycle_index].position_data,
        testing::ElementsAreArray(golden_position_config[clock_cycle_index]))
        << "Vectors differ at clock cycle: " << clock_cycle_index;
  }
}

TEST(DMACrossbarSetupTest, RecordSize18BufferToInterfaceSetupCheck) {
  const int record_size = 18;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = true;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize18BufferToInterfaceChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize18BufferToInterfacePositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize18InterfaceToBufferSetupCheck) {
  const int record_size = 18;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = false;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize18InterfaceToBufferChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize18InterfaceToBufferPositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize4BufferToInterfaceSetupCheck) {
  const int record_size = 4;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = true;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize4BufferToInterfaceChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize4BufferToInterfacePositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize4InterfaceToBufferSetupCheck) {
  const int record_size = 4;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = false;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize4InterfaceToBufferChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize4InterfaceToBufferPositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize46BufferToInterfaceSetupCheck) {
  int record_size = 46;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = true;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize46BufferToInterfaceChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize46BufferToInterfacePositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize46InterfaceToBufferSetupCheck) {
  int record_size = 46;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = false;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize46InterfaceToBufferChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize46InterfaceToBufferPositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize57BufferToInterfaceSetupCheck) {
  int record_size = 57;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = true;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize57BufferToInterfaceChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize57BufferToInterfacePositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize57InterfaceToBufferSetupCheck) {
  int record_size = 57;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = false;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize57InterfaceToBufferChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize57InterfaceToBufferPositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize510BufferToInterfaceSetupCheck) {
  const int record_size = 510;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = true;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize510BufferToInterfaceChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize510BufferToInterfacePositionSetup.txt");
}

TEST(DMACrossbarSetupTest, RecordSize510InterfaceToBufferSetupCheck) {
  const int record_size = 510;
  DMASetupData test_stream_setup_data;
  test_stream_setup_data.chunks_per_record =
      CalculateChunksPerRecord(record_size);
  test_stream_setup_data.is_input_stream = false;
  ExpectConfigurationDataIsUnconfigured(test_stream_setup_data);

  DMACrossbarSetup::CalculateCrossbarSetupData(
      kTestAnyChunk, kTestAnyPosition, test_stream_setup_data, record_size);

  ExpectConfigurationDataIsConfigured(
      test_stream_setup_data,
      "DMACrossbarSetupTest/RecordSize510InterfaceToBufferChunkSetup.txt",
      "DMACrossbarSetupTest/RecordSize510InterfaceToBufferPositionSetup.txt");
}

}  // namespace
