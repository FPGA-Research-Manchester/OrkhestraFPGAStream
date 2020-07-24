#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockDMA.hpp"
#include "DMASetup.hpp"
#include <cstdio>
namespace {
	TEST(DMASetupTest, InputParamsSettings) {
		int expectedInputStreamID = 0;
		int expectedDDRBurstLength = 72;
		int expectedRecordsPerDDRBurst = 16;
		int expectedBufferStart = 0;
		int expectedBufferEnd = 15;
		int expectedStreamSize = 1000;
		std::vector<int> mockDBData(1, 0);
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setInputControllerParams(expectedInputStreamID, expectedDDRBurstLength, expectedRecordsPerDDRBurst, expectedBufferStart, expectedBufferEnd)).Times(1);
		EXPECT_CALL(mockDMA, setInputControllerStreamAddress(expectedInputStreamID, reinterpret_cast<uintptr_t>(&mockDBData[0]))).Times(1);
		EXPECT_CALL(mockDMA, setInputControllerStreamSize(expectedInputStreamID, expectedStreamSize)).Times(1);
		DMASetup DMAConfigurer;
		
		DMAConfigurer.SetupDMAModule(expectedStreamSize, mockDBData, 18, mockDMA);
	}
	TEST(DMASetupTest, OutputParamsSettings) {
		int expectedOutputStreamID = 1;
		int expectedDDRBurstLength = 72;
		int expectedRecordsPerDDRBurst = 16;
		int expectedBufferStart = 0;
		int expectedBufferEnd = 15;
		std::vector<int> mockDBData(1, 0);
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setOutputControllerParams(expectedOutputStreamID, expectedDDRBurstLength, expectedRecordsPerDDRBurst, expectedBufferStart, expectedBufferEnd)).Times(1);
		EXPECT_CALL(mockDMA, setOutputControllerStreamAddress(expectedOutputStreamID, reinterpret_cast<uintptr_t>(&mockDBData[0]))).Times(1);
		EXPECT_CALL(mockDMA, setOutputControllerStreamSize(expectedOutputStreamID, 0)).Times(1);
		DMASetup DMAConfigurer;
		DMAConfigurer.SetupDMAModule(1000, mockDBData, 18, mockDMA);
	}
	TEST(DMASetupTest, RecordSettings) {
		int expectedOutputStreamID = 1;
		int expectedInputStreamID = 0;
		std::vector<int> mockDBData(1, 0);
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setRecordSize(expectedOutputStreamID, 2)).Times(1);
		EXPECT_CALL(mockDMA, setRecordSize(expectedInputStreamID, 2)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(expectedOutputStreamID, 0, 0)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(expectedOutputStreamID, 1, 1)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(expectedInputStreamID, 0, 0)).Times(1);
		EXPECT_CALL(mockDMA, setRecordChunkIDs(expectedInputStreamID, 1, 1)).Times(1);
		DMASetup DMAConfigurer;
		DMAConfigurer.SetupDMAModule(1000, mockDBData, 18, mockDMA);
	}
}