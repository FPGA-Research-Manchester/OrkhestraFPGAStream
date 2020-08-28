#include "filter_setup.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_filter.hpp"
namespace {
const int kExpectedChunkId = 1;
const int kExpectedPosition = 14;
const int kInputStreamId = 0;
const int kOutputStreamId = 1;

TEST(FilterSetupTest, FilterStreamsSetting) {
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter, FilterSetStreamIDs(kInputStreamId, kOutputStreamId,
                                              kOutputStreamId))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, kInputStreamId,
                                      kOutputStreamId);
}
TEST(FilterSetupTest, FilterModesSetting) {
  bool expected_request_on_invalid_if_last = true;
  bool expected_forward_invalid_record_first_chunk = false;
  bool expected_forward_full_invalid_records = false;

  bool expected_first_module_in_resource_elastic_chain = true;
  bool expected_last_module_in_resource_elastic_chain = true;

  MockFilter mock_filter;
  EXPECT_CALL(mock_filter,
              FilterSetMode(expected_request_on_invalid_if_last,
                            expected_forward_invalid_record_first_chunk,
                            expected_forward_full_invalid_records,
                            expected_first_module_in_resource_elastic_chain,
                            expected_last_module_in_resource_elastic_chain))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, kInputStreamId,
                                      kOutputStreamId);
}
TEST(FilterSetupTest, CompareTypesSetting) {
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter,
              FilterSetCompareTypes(
                  kExpectedChunkId, kExpectedPosition,
                  filter_config_values::CompareFunctions::kFilter32BitLessThan,
                  testing::_, testing::_, testing::_))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, kInputStreamId,
                                      kOutputStreamId);
}
TEST(FilterSetupTest, ReferenceValuesSetting) {
  int expected_compare_reference_value = 12000;
  int expected_compare_unit_index = 0;
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter, FilterSetCompareReferenceValue(
                               kExpectedChunkId, kExpectedPosition,
                               expected_compare_unit_index,
                               expected_compare_reference_value))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, kInputStreamId,
                                      kOutputStreamId);
}
TEST(FilterSetupTest, DNFClauseSetting) {
  int expected_compare_unit_index = 0;
  int expected_dnf_clause_id = 0;
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter,
              FilterSetDNFClauseLiteral(
                  expected_dnf_clause_id, expected_compare_unit_index,
                  kExpectedChunkId, kExpectedPosition,
                  filter_config_values::LiteralTypes::kLiteralPositive))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, kInputStreamId,
                                      kOutputStreamId);
}
TEST(FilterSetupTest, CorrectFilterCalled) {
  int expected_datapath_width = 16;
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter, WriteDNFClauseLiteralsToFilter_1CMP_8DNF(testing::_))
      .Times(0);
  EXPECT_CALL(mock_filter,
              WriteDNFClauseLiteralsToFilter_2CMP_16DNF(testing::_))
      .Times(0);
  EXPECT_CALL(mock_filter, WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
                               expected_datapath_width))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, kInputStreamId,
                                      kOutputStreamId);
}
}  // namespace