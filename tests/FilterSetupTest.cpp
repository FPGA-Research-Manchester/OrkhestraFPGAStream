#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "FilterSetup.hpp"
#include "MockFilter.hpp"
namespace {
int expected_chunk_id = 1;
int expected_position = 1;
int input_stream_id = 0;
int output_stream_id = 1;

TEST(FilterSetupTest, FilterStreamsSetting) {
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter, filterSetStreamIDs(input_stream_id, output_stream_id,
                                              output_stream_id))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, input_stream_id,
                                      output_stream_id);
}
TEST(FilterSetupTest, FilterModesSetting) {
  bool expected_request_on_invalid_if_last = true;
  bool expected_forward_invalid_record_first_chunk = false;
  bool expected_forward_full_invalid_records = false;

  bool expected_first_module_in_resource_elastic_chain = true;
  bool expected_last_module_in_resource_elastic_chain = true;

  MockFilter mock_filter;
  EXPECT_CALL(mock_filter,
              filterSetMode(expected_request_on_invalid_if_last,
                            expected_forward_invalid_record_first_chunk,
                            expected_forward_full_invalid_records,
                            expected_first_module_in_resource_elastic_chain,
                            expected_last_module_in_resource_elastic_chain))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, input_stream_id,
                                      output_stream_id);
}
TEST(FilterSetupTest, CompareTypesSetting) {
  int expected_less_than_compare = 0;
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter,
              filterSetCompareTypes(expected_chunk_id, expected_position,
                                    expected_less_than_compare, testing::_,
                                    testing::_, testing::_))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, input_stream_id,
                                      output_stream_id);
}
TEST(FilterSetupTest, ReferenceValuesSetting) {
  int expected_compare_reference_value = 12000;
  int expected_compare_unit_index = 0;
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter, filterSetCompareReferenceValue(
                               expected_chunk_id, expected_position,
                               expected_compare_unit_index,
                               expected_compare_reference_value))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, input_stream_id,
                                      output_stream_id);
}
TEST(FilterSetupTest, DNFClauseSetting) {
  int expected_compare_unit_index = 0;
  int expected_dnf_clause_id = 0;
  int expected_positive_literal_type = 1;
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter,
              filterSetDNFClauseLiteral(expected_dnf_clause_id,
                                        expected_compare_unit_index,
                                        expected_chunk_id, expected_position,
                                        expected_positive_literal_type))
      .Times(1);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, input_stream_id,
                                      output_stream_id);
}
TEST(FilterSetupTest, CorrectFilterCalled) {
  int expected_datapath_width = 16;
  MockFilter mock_filter;
  EXPECT_CALL(mock_filter,
              writeDNFClauseLiteralsToFilter_1CMP_8DNF(expected_datapath_width))
      .Times(1);
  EXPECT_CALL(mock_filter,
              writeDNFClauseLiteralsToFilter_2CMP_16DNF(testing::_))
      .Times(0);
  EXPECT_CALL(mock_filter,
              writeDNFClauseLiteralsToFilter_4CMP_32DNF(testing::_))
      .Times(0);
  FilterSetup filter_configurer;
  filter_configurer.SetupFilterModule(mock_filter, input_stream_id,
                                      output_stream_id);
}
}  // namespace