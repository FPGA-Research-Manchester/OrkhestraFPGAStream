#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockFilter.hpp"
#include "FilterSetup.hpp"
namespace {
	int expectedChunkID = 1;
	int expectedPosition = 1;
	int inputStreamID = 0;
	int outputStreamID = 1;

	TEST(FilterSetupTest, FilterStreamsSetting) {
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetStreamIDs(inputStreamID, outputStreamID, outputStreamID)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter, inputStreamID, outputStreamID);
	}
	TEST(FilterSetupTest, FilterModesSetting) {

		bool expectedRequestOnInvalidIfLast = true;
		bool expectedForwardInvalidRecordFirstChunk = false;
		bool expectedForwardFullInvalidRecords = false;

		bool expectedFirstModuleInResourceElasticChain = true;
		bool expectedLastModuleInResourceElasticChain = true;

		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetMode(expectedRequestOnInvalidIfLast, expectedForwardInvalidRecordFirstChunk, expectedForwardFullInvalidRecords, expectedFirstModuleInResourceElasticChain, expectedLastModuleInResourceElasticChain)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter, inputStreamID, outputStreamID);
	}
	TEST(FilterSetupTest, CompareTypesSetting) {
		int expectedLessThanCompare = 0;
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetCompareTypes(expectedChunkID, expectedPosition, expectedLessThanCompare, testing::_, testing::_, testing::_)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter, inputStreamID, outputStreamID);
	}
	TEST(FilterSetupTest, ReferenceValuesSetting) {
		int expectedCompareReferenceValue = 12000;
		int expectedCompareUnitIndex = 0;
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetCompareReferenceValue(expectedChunkID, expectedPosition, expectedCompareUnitIndex, expectedCompareReferenceValue)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter, inputStreamID, outputStreamID);
	}
	TEST(FilterSetupTest, DNFClauseSetting) {
		int expectedCompareUnitIndex = 0;
		int expectedDNFClauseID = 0;
		int expectedPositiveLiteralType = 1;
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetDNFClauseLiteral(expectedDNFClauseID, expectedCompareUnitIndex, expectedChunkID, expectedPosition, expectedPositiveLiteralType)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter, inputStreamID, outputStreamID);
	}
	TEST(FilterSetupTest, CorrectFilterCalled) {
		int expectedDatapathWidth = 16;
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, writeDNFClauseLiteralsToFilter_1CMP_8DNF(expectedDatapathWidth)).Times(1);
		EXPECT_CALL(mockFilter, writeDNFClauseLiteralsToFilter_2CMP_16DNF(testing::_)).Times(0);
		EXPECT_CALL(mockFilter, writeDNFClauseLiteralsToFilter_4CMP_32DNF(testing::_)).Times(0);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter, inputStreamID, outputStreamID);
	}
}