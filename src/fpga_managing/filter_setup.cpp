#include "filter_setup.hpp"

#include <cstdio>
#include <stdexcept>

#include "query_acceleration_constants.hpp"

// Car filtering
void FilterSetup::SetupFilterModuleCars(FilterInterface& filter_module,
                                        const int input_stream_id,
                                        const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);
  SetComparisons(filter_module,
                 {{filter_config_values::CompareFunctions::kFilter32BitLessThan,
                   {12000},
                   {filter_config_values::LiteralTypes::kLiteralPositive},
                   {0}}},
                 1, 14);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

// SELECT * FROM part
// WHERE((p_size >= 1) AND(((p_brand = 'Brand#12' ::bpchar) AND(
//    p_container = ANY('{"SM CASE","SM BOX","SM PACK","SM PKG"}' ::bpchar[]))
//                        AND(p_size <= 5))))
// Needs more OR cases for Q19. But that needs data duplication
void FilterSetup::SetupFilterModulePartQ19(FilterInterface& filter_module,
                                           const int input_stream_id,
                                           const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);
  // p_size
  SetComparisons(
      filter_module,
      {{filter_config_values::CompareFunctions::kFilter32BitLessThanOrEqual,
        {5},
        {filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive},
        {0, 1, 2, 3}},
       {filter_config_values::CompareFunctions::kFilter32BitGreaterThanOrEqual,
        {1},
        {filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive},
        {0, 1, 2, 3}}},
      2, 15);

  // p_brand
  SetComparisons(filter_module,
                 {{filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("Brand#12  ", 3),
                   {filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive},
                   {0, 1, 2, 3}}},
                 1, 9);

  // p_container
  SetComparisons(filter_module,
                 {{filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("SM CASE   ", 3),
                   {filter_config_values::LiteralTypes::kLiteralPositive},
                   {0}},
                  {filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("SM BOX    ", 3),
                   {filter_config_values::LiteralTypes::kLiteralPositive},
                   {1}},
                  {filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("SM PACK   ", 3),
                   {filter_config_values::LiteralTypes::kLiteralPositive},
                   {2}},
                  {filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("SM PKG    ", 3),
                   {filter_config_values::LiteralTypes::kLiteralPositive},
                   {3}}},
                 2, 14);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

// SELECT * FROM lineitem WHERE (l_shipmode = ANY ('{AIR,"AIR REG"}'::bpchar[]))
// AND (l_shipinstruct = 'DELIVER IN PERSON'::bpchar) AND (((l_quantity >=
// '1'::numeric) AND (l_quantity <= '11'::numeric)) OR ((l_quantity >=
// '10'::numeric) AND (l_quantity <= '20'::numeric)) OR ((l_quantity >=
// '20'::numeric) AND (l_quantity <= '30'::numeric)))
void FilterSetup::SetupFilterModuleLineitemQ19(FilterInterface& filter_module,
                                               const int input_stream_id,
                                               const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);
  // l_quantity
  // Since this module only has 4 compare lanes we will combine the query
  // l_quantity comparisons
  SetComparisons(
      filter_module,
      {{filter_config_values::CompareFunctions::kFilter32BitLessThanOrEqual,
        {30 * 100},
        {filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive},
        {0, 1, 2, 3}},
       {filter_config_values::CompareFunctions::kFilter32BitGreaterThanOrEqual,
        {1 * 100},
        {filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralDontCare,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralDontCare},
        {0, 1, 2, 3}}},
      0, 10);
  SetComparisons(
      filter_module,
      {{filter_config_values::CompareFunctions::kFilter32BitEqual,
        {0},
        {filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralPositive},
        {0, 1, 2, 3}},
       {filter_config_values::CompareFunctions::kFilter32BitGreaterThan,
        {1},
        {filter_config_values::LiteralTypes::kLiteralDontCare,
         filter_config_values::LiteralTypes::kLiteralPositive,
         filter_config_values::LiteralTypes::kLiteralDontCare,
         filter_config_values::LiteralTypes::kLiteralPositive},
        {0, 1, 2, 3}}},
      0, 11);

  // l_shipmode
  // The table only has REG AIR but we still keep the orignally generated query
  SetComparisons(filter_module,
                 {{filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("AIR REG   ", 3),
                   {filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralDontCare,
                    filter_config_values::LiteralTypes::kLiteralDontCare},
                   {0, 1, 2, 3}},
                  {filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("AIR       ", 3),
                   {filter_config_values::LiteralTypes::kLiteralDontCare,
                    filter_config_values::LiteralTypes::kLiteralDontCare,
                    filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive},
                   {0, 1, 2, 3}}},
                 1, 7);

  // l_shipinstruct
  SetComparisons(filter_module,
                 {{filter_config_values::CompareFunctions::kFilter32BitEqual,
                   ConvertCharStringToAscii("DELIVER IN PERSON        ", 7),
                   {filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive,
                    filter_config_values::LiteralTypes::kLiteralPositive},
                   {0, 1, 2, 3}}},
                 1, 14);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

void FilterSetup::SetOneOutputSingleModuleMode(FilterInterface& filter_module) {
  bool request_on_invalid_if_last = true;
  bool forward_invalid_record_first_chunk = false;
  bool forward_full_invalid_records = false;

  bool first_module_in_resource_elastic_chain = true;
  bool last_module_in_resource_elastic_chain = true;

  filter_module.FilterSetMode(
      request_on_invalid_if_last, forward_invalid_record_first_chunk,
      forward_full_invalid_records, first_module_in_resource_elastic_chain,
      last_module_in_resource_elastic_chain);
}

void FilterSetup::SetComparisons(FilterInterface& filter_module,
                                 std::vector<FilterComparison> comparisons,
                                 int chunk_id, int data_position) {
  std::vector<filter_config_values::CompareFunctions> compare_functions;
  for (int compare_lane_index = 0; compare_lane_index < comparisons.size();
       compare_lane_index++) {
    for (int compare_value_index = 0;
         compare_value_index <
         comparisons[compare_lane_index].compare_reference_values.size();
         compare_value_index++) {
      int current_chunk_id =
          chunk_id + ((15 - data_position + compare_value_index) / 16);
      int current_data_position =
          15 - ((15 - data_position + compare_value_index) % 16);
      filter_module.FilterSetCompareReferenceValue(
          current_chunk_id, current_data_position, compare_lane_index,
          comparisons[compare_lane_index]
              .compare_reference_values[compare_value_index]);
      for (int dnf = 0;
           dnf < comparisons[compare_lane_index].dnf_clause_ids.size(); dnf++) {
        filter_module.FilterSetDNFClauseLiteral(
            comparisons[compare_lane_index].dnf_clause_ids[dnf],
            compare_lane_index, current_chunk_id, current_data_position,
            comparisons[compare_lane_index].literal_types[dnf]);
      }
    }
    compare_functions.push_back(
        comparisons[compare_lane_index].compare_function);
  }

  for (int leftover_compare_lane_id = comparisons.size();
       leftover_compare_lane_id < 4; leftover_compare_lane_id++) {
    // Set random comparisons for unused lanes.
    compare_functions.push_back(
        filter_config_values::CompareFunctions::kFilter32BitEqual);
  }

  for (int compare_value_index = 0;
       compare_value_index < comparisons[0].compare_reference_values.size();
       compare_value_index++) {
    int current_chunk_id =
        chunk_id + ((15 - data_position + compare_value_index) / 16);
    int current_data_position =
        15 - ((15 - data_position + compare_value_index) % 16);
    filter_module.FilterSetCompareTypes(
        current_chunk_id, current_data_position, compare_functions[0],
        compare_functions[1], compare_functions[2], compare_functions[3]);
  }
}

auto FilterSetup::ConvertCharStringToAscii(const std::string& input_string,
                                           int output_size)
    -> std::vector<int> {
  if (input_string.length() > output_size * 4) {
    throw std::runtime_error(
        (input_string + " is longer than " + std::to_string(output_size * 4))
            .c_str());
  }
  std::vector<int> integer_values(output_size, 0);
  for (int i = 0; i < input_string.length(); i++) {
    integer_values[i / 4] += int(input_string[i]) << (3 - (i % 4)) * 8;
  }
  return integer_values;
}