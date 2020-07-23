#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockDMA.hpp"
#include "DMASetup.hpp"
namespace {
	TEST(DMASetupTest, InputParamsSettings) {
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setInputControllerParams(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1);
		EXPECT_CALL(mockDMA, setInputControllerStreamAddress(testing::_, testing::_)).Times(1);
		EXPECT_CALL(mockDMA, setInputControllerStreamSize(testing::_, testing::_)).Times(1);
		DMASetup DMAConfigurer;
		std::vector<int> mockDBData(10,0);
		DMAConfigurer.SetupDMAModule(1, mockDBData, 1, mockDMA);
	}
	TEST(DMASetupTest, OutputParamsSettings) {
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setOutputControllerParams(testing::_, testing::_, testing::_, testing::_, testing::_)).Times(1);
		EXPECT_CALL(mockDMA, setOutputControllerStreamAddress(testing::_, testing::_)).Times(1);
		EXPECT_CALL(mockDMA, setOutputControllerStreamSize(testing::_, testing::_)).Times(1);
		DMASetup DMAConfigurer;
		std::vector<int> mockDBData(10, 0);
		DMAConfigurer.SetupDMAModule(1, mockDBData, 1, mockDMA);
	}
	TEST(DMASetupTest, RecordSettings) {
		MockDMA mockDMA;
		EXPECT_CALL(mockDMA, setRecordSize(testing::_, testing::_)).Times(2);
		//EXPECT_CALL(mockDMA, setRecordChunkIDs(testing::_, testing::_, testing::_));
		DMASetup DMAConfigurer;
		std::vector<int> mockDBData(10, 0);
		DMAConfigurer.SetupDMAModule(1, mockDBData, 1, mockDMA);
	}
}