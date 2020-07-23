#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockFilter.hpp"
#include "FilterSetup.hpp"
namespace {
	TEST(FilterSetupTest, FilterStreamsSetting) {
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetStreamIDs(0, 1, 0)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter);
	}
	TEST(FilterSetupTest, FilterModesSetting) {
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetMode(true, false, false, true, true)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter);
	}
	TEST(FilterSetupTest, CompareTypesSetting) {
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetCompareTypes(1, 1, 0, 0, 0, 0)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter);
	}
	TEST(FilterSetupTest, ReferenceValuesSetting) {
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetCompareReferenceValue(1, 1, 1, 12000)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter);
	}
	TEST(FilterSetupTest, DNFClauseSetting) {
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, filterSetDNFClauseLiteral(0, 0, 1, 1, 1)).Times(1);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter);
	}
	TEST(FilterSetupTest, CorrectFilterCalled) {
		MockFilter mockFilter;
		EXPECT_CALL(mockFilter, writeDNFClauseLiteralsToFilter_1CMP_8DNF(16)).Times(1);
		EXPECT_CALL(mockFilter, writeDNFClauseLiteralsToFilter_2CMP_16DNF(testing::_)).Times(0);
		EXPECT_CALL(mockFilter, writeDNFClauseLiteralsToFilter_4CMP_32DNF(testing::_)).Times(0);
		FilterSetup filterConfigurer;
		filterConfigurer.SetupFilterModule(mockFilter);
	}
}