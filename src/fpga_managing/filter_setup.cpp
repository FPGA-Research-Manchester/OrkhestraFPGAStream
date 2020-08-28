#include "filter_setup.hpp"
#include "query_acceleration_constants.hpp"

#include <cstdio>

#include "filter_configuration_values.hpp"

void FilterSetup::SetupFilterModule(FilterInterface& filter_module,
                                    const int input_stream_id,
                                    const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  bool request_on_invalid_if_last = true;
  bool forward_invalid_record_first_chunk = false;
  bool forward_full_invalid_records = false;

  bool first_module_in_resource_elastic_chain = true;
  bool last_module_in_resource_elastic_chain = true;

  filter_module.FilterSetMode(
      request_on_invalid_if_last, forward_invalid_record_first_chunk,
      forward_full_invalid_records, first_module_in_resource_elastic_chain,
      last_module_in_resource_elastic_chain);

  int chunk_id = 1;
  int data_position = 14;
  
  filter_config_values::CompareFunctions any_compare =
      filter_config_values::CompareFunctions::kFilter32BitEqual;

  filter_module.FilterSetCompareTypes(chunk_id, data_position,
      filter_config_values::CompareFunctions::kFilter32BitLessThan, any_compare,
      any_compare, any_compare);

  int compare_lane_index = 0;
  int compare_reference_value = 12000;

  filter_module.FilterSetCompareReferenceValue(
      chunk_id, data_position, compare_lane_index, compare_reference_value);

  int dnf_clause_id = 0;

  filter_module.FilterSetDNFClauseLiteral(
      dnf_clause_id, compare_lane_index, chunk_id, data_position,
      filter_config_values::LiteralTypes::kLiteralPositive);

  // Currently 4CMP_32DNF module is hardcoded in
  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}
