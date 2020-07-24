#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockDMA.hpp"
#include "DMASetup.hpp"
#include <cstdio>
namespace {
	int inputStreamID = 0;
	int outputStreamID = 1;

	TEST(DMASetupTest, InputParamsSettings) {
		int expectedDDRBurstLength = 72;
		int expectedRecordsPerDDRBurst = 16;
		int expectedBufferStart = 0;
		int expectedBufferEnd = 15;
		int expectedStreamSize = 1000;
		std::vector<int> mockDBData(1, 0);
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setInputControllerParams(inputStreamID, expectedDDRBurstLength, expectedRecordsPerDDRBurst, expectedBufferStart, expectedBufferEnd)).Times(1);
		EXPECT_CALL(mockDMA, setInputControllerStreamAddress(inputStreamID, reinterpret_cast<uintptr_t>(&mockDBData[0]))).Times(1);
		EXPECT_CALL(mockDMA, setInputControllerStreamSize(inputStreamID, expectedStreamSize)).Times(1);
		DMASetup DMAConfigurer;
		
		DMAConfigurer.SetupDMAModule(mockDMA, mockDBData, 18, expectedStreamSize, inputStreamID, outputStreamID);
	}
	TEST(DMASetupTest, OutputParamsSettings) {
		int expectedDDRBurstLength = 72;
		int expectedRecordsPerDDRBurst = 16;
		int expectedBufferStart = 0;
		int expectedBufferEnd = 15;
		std::vector<int> mockDBData(1, 0);
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setOutputControllerParams(outputStreamID, expectedDDRBurstLength, expectedRecordsPerDDRBurst, expectedBufferStart, expectedBufferEnd)).Times(1);
		EXPECT_CALL(mockDMA, setOutputControllerStreamAddress(outputStreamID, reinterpret_cast<uintptr_t>(&mockDBData[0]))).Times(1);
		EXPECT_CALL(mockDMA, setOutputControllerStreamSize(outputStreamID, 0)).Times(1);
		DMASetup DMAConfigurer;
		DMAConfigurer.SetupDMAModule(mockDMA, mockDBData, 18, 1000, inputStreamID, outputStreamID);
	}
	TEST(DMASetupTest, RecordSettings) {
		std::vector<int> mockDBData(1, 0);
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setRecordSize(outputStreamID, 2)).Times(1);
		EXPECT_CALL(mockDMA, setRecordSize(inputStreamID, 2)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(outputStreamID, 0, 0)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(outputStreamID, 1, 1)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(inputStreamID, 0, 0)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(inputStreamID, 1, 1)).Times(1);
		DMASetup DMAConfigurer;
		DMAConfigurer.SetupDMAModule(mockDMA, mockDBData, 18, 1000, inputStreamID, outputStreamID);
	}
}