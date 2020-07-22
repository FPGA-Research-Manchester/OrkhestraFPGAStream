#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "DMACrossbarSetup.hpp"
#include "DMASetupData.hpp"
#include <fstream>
#include <sstream>
#include <string>
namespace {
	const int TEST_ANY_CHUNK = 0;
	const int TEST_ANY_POSITION = 1;
	const std::vector<int> UNTOUCHED_VECTOR(16, 0);

	void GetGoldenConfigFromFile(std::vector<std::vector<int>>& goldenConfig, std::string fileName) {
		std::ifstream inputFile(fileName);
		ASSERT_TRUE(inputFile);

		std::string line;
		while (std::getline(inputFile, line))
		{
			std::istringstream stringStream(line);
			int configValue;
			std::vector<int> currentCycleGoldenConfig;
			while (stringStream >> configValue) {
				currentCycleGoldenConfig.push_back(configValue);
			}
			goldenConfig.push_back(currentCycleGoldenConfig);
		}
	}

	TEST(DMACrossbarSetupTest, BufferToInterfaceHasCorrectChunkSetup) {
		DMASetupData testStreamSetupData;
		DMACrossbarSetup crossbarConfigurerUnderTest;
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].chunkData, testing::ElementsAreArray(UNTOUCHED_VECTOR));
		}
		crossbarConfigurerUnderTest.FindInputCrossbarSetupData(TEST_ANY_CHUNK, TEST_ANY_POSITION, testStreamSetupData);
		std::vector<std::vector<int>> goldenConfig;
		GetGoldenConfigFromFile(goldenConfig, "DMACrossbarSetupTest/GoldenBufferToInterfaceChunkSetup.txt");
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].chunkData, testing::ElementsAreArray(goldenConfig[clockCycleIndex]));
		}
	}
	TEST(DMACrossbarSetupTest, BufferToInterfaceHasCorrectPositionSetup) {
		DMASetupData testStreamSetupData;
		DMACrossbarSetup crossbarConfigurerUnderTest;
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].positionData, testing::ElementsAreArray(UNTOUCHED_VECTOR));
		}
		crossbarConfigurerUnderTest.FindInputCrossbarSetupData(TEST_ANY_CHUNK, TEST_ANY_POSITION, testStreamSetupData);
		std::vector<std::vector<int>> goldenConfig;
		GetGoldenConfigFromFile(goldenConfig, "DMACrossbarSetupTest/GoldenBufferToInterfacePositionSetup.txt");
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].positionData, testing::ElementsAreArray(goldenConfig[clockCycleIndex]));
		}
	}
	TEST(DMACrossbarSetupTest, InterfaceToBufferHasCorrectChunkSetup) {
		DMASetupData testStreamSetupData;
		DMACrossbarSetup crossbarConfigurerUnderTest;
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].chunkData, testing::ElementsAreArray(UNTOUCHED_VECTOR));
		}
		crossbarConfigurerUnderTest.FindOutputCrossbarSetupData(TEST_ANY_CHUNK, TEST_ANY_POSITION, testStreamSetupData);
		std::vector<std::vector<int>> goldenConfig;
		GetGoldenConfigFromFile(goldenConfig, "DMACrossbarSetupTest/GoldenInterfaceToBufferChunkSetup.txt");
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].chunkData, testing::ElementsAreArray(goldenConfig[clockCycleIndex]));
		}
	}
	TEST(DMACrossbarSetupTest, InterfaceToBufferHasCorrectPositionSetup) {
		DMASetupData testStreamSetupData;
		DMACrossbarSetup crossbarConfigurerUnderTest;
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].positionData, testing::ElementsAreArray(UNTOUCHED_VECTOR));
		}
		crossbarConfigurerUnderTest.FindOutputCrossbarSetupData(TEST_ANY_CHUNK, TEST_ANY_POSITION, testStreamSetupData);
		std::vector<std::vector<int>> goldenConfig;
		GetGoldenConfigFromFile(goldenConfig, "DMACrossbarSetupTest/GoldenInterfaceToBufferPositionSetup.txt");
		for (int clockCycleIndex = 0; clockCycleIndex < 32; clockCycleIndex++) {
			EXPECT_THAT(testStreamSetupData.crossbarSetupData[clockCycleIndex].positionData, testing::ElementsAreArray(goldenConfig[clockCycleIndex]));
		}
	}
}
