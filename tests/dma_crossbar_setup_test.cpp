#include "dma_crossbar_setup.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#include "dma_setup_data.hpp"
namespace {
const int kTestAnyChunk = 0;
const int kTestAnyPosition = 1;
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

TEST(DMACrossbarSetupTest, BufferToInterfaceHasCorrectChunkSetup) {
  DMASetupData test_stream_setup_data;
  DMACrossbarSetup crossbar_configurer_under_test;
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .chunk_data,
                testing::ElementsAreArray(kUntouchedVector));
  }
  DMACrossbarSetup::FindInputCrossbarSetupData(kTestAnyChunk, kTestAnyPosition,
                                               test_stream_setup_data);
  std::vector<std::vector<int>> golden_config;
  GetGoldenConfigFromFile(
      golden_config,
      "DMACrossbarSetupTest/GoldenBufferToInterfaceChunkSetup.txt");
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .chunk_data,
                testing::ElementsAreArray(golden_config[clock_cycle_index]));
  }
}
TEST(DMACrossbarSetupTest, BufferToInterfaceHasCorrectPositionSetup) {
  DMASetupData test_stream_setup_data;
  DMACrossbarSetup crossbar_configurer_under_test;
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .position_data,
                testing::ElementsAreArray(kUntouchedVector));
  }
  DMACrossbarSetup::FindInputCrossbarSetupData(kTestAnyChunk, kTestAnyPosition,
                                               test_stream_setup_data);
  std::vector<std::vector<int>> golden_config;
  GetGoldenConfigFromFile(
      golden_config,
      "DMACrossbarSetupTest/GoldenBufferToInterfacePositionSetup.txt");
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .position_data,
                testing::ElementsAreArray(golden_config[clock_cycle_index]));
  }
}
TEST(DMACrossbarSetupTest, InterfaceToBufferHasCorrectChunkSetup) {
  DMASetupData test_stream_setup_data;
  DMACrossbarSetup crossbar_configurer_under_test;
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .chunk_data,
                testing::ElementsAreArray(kUntouchedVector));
  }
  DMACrossbarSetup::FindOutputCrossbarSetupData(kTestAnyChunk, kTestAnyPosition,
                                                test_stream_setup_data);
  std::vector<std::vector<int>> golden_config;
  GetGoldenConfigFromFile(
      golden_config,
      "DMACrossbarSetupTest/GoldenInterfaceToBufferChunkSetup.txt");
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .chunk_data,
                testing::ElementsAreArray(golden_config[clock_cycle_index]));
  }
}
TEST(DMACrossbarSetupTest, InterfaceToBufferHasCorrectPositionSetup) {
  DMASetupData test_stream_setup_data;
  DMACrossbarSetup crossbar_configurer_under_test;
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .position_data,
                testing::ElementsAreArray(kUntouchedVector));
  }
  DMACrossbarSetup::FindOutputCrossbarSetupData(kTestAnyChunk, kTestAnyPosition,
                                                test_stream_setup_data);
  std::vector<std::vector<int>> golden_config;
  GetGoldenConfigFromFile(
      golden_config,
      "DMACrossbarSetupTest/GoldenInterfaceToBufferPositionSetup.txt");
  for (int clock_cycle_index = 0; clock_cycle_index < 32; clock_cycle_index++) {
    EXPECT_THAT(test_stream_setup_data.crossbar_setup_data[clock_cycle_index]
                    .position_data,
                testing::ElementsAreArray(golden_config[clock_cycle_index]));
  }
}
}  // namespace
