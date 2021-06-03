#include "filter_setup.hpp"

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "query_acceleration_constants.hpp"

using namespace dbmstodspi::fpga_managing;

void FilterSetup::SetupFilterModule(
    modules::FilterInterface& filter_module, int input_stream_id,
    int output_stream_id,
    const std::vector<std::vector<int>>& operation_parameters) {
  if (operation_parameters.empty() || operation_parameters.at(0).empty()) {
    throw std::runtime_error("No parameters given!");
  }
  filter_module.ResetDNFStates();
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);

  SetAllComparisons(filter_module, operation_parameters);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);

  // switch (operation_parameters.at(0).at(0)) {
  //  case 0:
  //    SetupFilterModuleCars(filter_module, input_stream_id, output_stream_id);
  //    break;
  //  case 1:
  //    SetupFilterModuleLineitemQ19(filter_module, input_stream_id,
  //                                 output_stream_id);
  //    break;
  //  case 2:
  //    SetupFilterModulePartQ19(filter_module, input_stream_id,
  //                             output_stream_id);
  //    break;
  //  case 3:
  //    SetupFilterModuleFinalQ19(filter_module, input_stream_id,
  //                              output_stream_id);
  //    break;
  //  case 4:
  //    SetupFilterModuleFinalDoubleQ19(filter_module, input_stream_id,
  //                                    output_stream_id);
  //    break;
  //  default:
  //    throw std::runtime_error("Wrong parameters given!");
  //}
}

void FilterSetup::SetAllComparisons(
    modules::FilterInterface& filter_module,
    const std::vector<std::vector<int>>& operation_parameters) {
  int chunk_id_index = 0;
  int data_position_index = 1;
  int comparison_count_index = 2;
  int number_of_parameter_vectors = 4;

  int current_vector_id = 0;
  while (current_vector_id != operation_parameters.size()) {
    int current_chunk_id =
        operation_parameters.at(current_vector_id).at(chunk_id_index);
    int current_data_position =
        operation_parameters.at(current_vector_id).at(data_position_index);
    int comparison_count =
        operation_parameters.at(current_vector_id).at(comparison_count_index);
    std::vector<FilterComparison> current_comparison_parameters;
    for (int i = 0; i < comparison_count * number_of_parameter_vectors;
         i += number_of_parameter_vectors) {
      current_vector_id += i;

      auto compare_function =
          static_cast<module_config_values::FilterCompareFunctions>(
              operation_parameters.at(current_vector_id + 1).at(0));
      std::vector<int> compare_reference_values =
          operation_parameters.at(current_vector_id + 2);
      std::vector<module_config_values::LiteralTypes> literal_types;
      for (const auto& literal_type :
           operation_parameters.at(current_vector_id + 3)) {
        literal_types.push_back(
            static_cast<module_config_values::LiteralTypes>(literal_type));
      }
      std::vector<int> dnf_clause_ids =
          operation_parameters.at(current_vector_id + 4);

      current_comparison_parameters.emplace_back(compare_function,
                                                 compare_reference_values,
                                                 literal_types, dnf_clause_ids);
    }
    SetComparisons(filter_module, current_comparison_parameters,
                   current_chunk_id, current_data_position);
    current_vector_id += number_of_parameter_vectors + 1;
  }
}

// Car filtering
void FilterSetup::SetupFilterModuleCars(modules::FilterInterface& filter_module,
                                        const int input_stream_id,
                                        const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kLessThan32Bit,
                   {12000},
                   {module_config_values::LiteralTypes::kLiteralPositive},
                   {0}}},
                 1, 14);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

// (p_brand = �Brand#12� AND l_quantity between 1 and 11) OR
// (p_brand = �Brand #23� AND l_quantity between 10 and 20) OR
// (p_brand = �Brand #34� AND l_quantity between 20 and 30)
void FilterSetup::SetupFilterModuleFinalQ19(
    modules::FilterInterface& filter_module, const int input_stream_id,
    const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);

  // l_quantity
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {11 * 100},
        {},
        {0}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {20 * 100},
        {},
        {1}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {30 * 100},
        {},
        {2}}},
      0, 14);
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {1 * 100},
        {},
        {0}},
       {module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {10 * 100},
        {},
        {1}},
       {module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {20 * 100},
        {},
        {2}}},
      0, 5);
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   {0},
                   {},
                   {0, 1, 2}}},
                 0, 15);
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   {0},
                   {},
                   {0, 1, 2}}},
                 0, 6);

  // p_brand
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#12  ", 3),
                   {},
                   {0}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#23  ", 3),
                   {},
                   {1}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#34  ", 3),
                   {},
                   {2}}},
                 0, 9);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

void FilterSetup::SetupFilterModuleFinalDoubleQ19(
    modules::FilterInterface& filter_module, const int input_stream_id,
    const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);

  // l_quantity
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {11 * 100},
        {},
        {0, 1, 2, 3}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {20 * 100},
        {},
        {4, 5, 6, 7}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {30 * 100},
        {},
        {8, 9, 10, 11}}},
      0, 13);
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {1 * 100},
        {},
        {0, 1, 2, 3}},
       {module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {10 * 100},
        {},
        {4, 5, 6, 7}},
       {module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {20 * 100},
        {},
        {8, 9, 10, 11}}},
      0, 11);
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   {0},
                   {},
                   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}}},
                 0, 14);
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   {0},
                   {},
                   {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}}},
                 0, 12);

  // p_brand
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#12  ", 3),
                   {},
                   {0, 1, 2, 3}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#23  ", 3),
                   {},
                   {4, 5, 6, 7}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#34  ", 3),
                   {},
                   {8, 9, 10, 11}}},
                 0, 6);

  // p_size
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {5},
        {},
        {0, 1, 2, 3}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {10},
        {},
        {4, 5, 6, 7}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {15},
        {},
        {8, 9, 10, 11}},
       {module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {1},
        {},
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}}},
      1, 15);

  // p_container
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM CASE   ", 3),
                   {},
                   {0}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM BOX    ", 3),
                   {},
                   {1}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM PACK   ", 3),
                   {},
                   {2}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM PKG    ", 3),
                   {},
                   {3}}},
                 1, 14);

  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED BAG   ", 3),
                   {},
                   {4}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED BOX   ", 3),
                   {},
                   {5}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED PKG   ", 3),
                   {},
                   {6}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED PACK  ", 3),
                   {},
                   {7}}},
                 1, 11);

  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG CASE   ", 3),
                   {},
                   {8}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG BOX    ", 3),
                   {},
                   {9}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG PACK   ", 3),
                   {},
                   {10}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG PKG    ", 3),
                   {},
                   {11}}},
                 1, 8);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

// SELECT * FROM part
// WHERE(((p_size >= 1) AND (((p_brand = 'Brand#12'::bpchar) AND (p_container =
// ANY ('{"SM CASE","SM BOX","SM PACK","SM PKG"}'::bpchar[])) AND (p_size <= 5))
// OR ((p_brand = 'Brand#23'::bpchar) AND (p_container = ANY ('{"MED BAG","MED
// BOX","MED PKG","MED PACK"}'::bpchar[])) AND (p_size= <= 10))
// OR ((p_brand = 'Brand#34'::bpchar) AND (p_container = ANY ('{"LG CASE","LG
// BOX","LG PACK","LG PKG"}'::bpchar[])) AND (p_size <= 15)))))
// p_container is originally at 2, 14. Will be duplicated to 2, 4 and to 2, 1
void FilterSetup::SetupFilterModulePartQ19(
    modules::FilterInterface& filter_module, const int input_stream_id,
    const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  SetOneOutputSingleModuleMode(filter_module);

  // p_size
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {5},
        {},
        {0, 1, 2, 3}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {10},
        {},
        {4, 5, 6, 7}},
       {module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {15},
        {},
        {8, 9, 10, 11}},
       {module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {1},
        {},
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}}},
      1, 15);

  // p_brand
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#12  ", 3),
                   {},
                   {0, 1, 2, 3}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#23  ", 3),
                   {},
                   {4, 5, 6, 7}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("Brand#34  ", 3),
                   {},
                   {8, 9, 10, 11}}},
                 0, 14);

  // p_container
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM CASE   ", 3),
                   {},
                   {0}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM BOX    ", 3),
                   {},
                   {1}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM PACK   ", 3),
                   {},
                   {2}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("SM PKG    ", 3),
                   {},
                   {3}}},
                 1, 14);

  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED BAG   ", 3),
                   {},
                   {4}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED BOX   ", 3),
                   {},
                   {5}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED PKG   ", 3),
                   {},
                   {6}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("MED PACK   ", 3),
                   {},
                   {7}}},
                 1, 11);

  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG CASE   ", 3),
                   {},
                   {8}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG BOX    ", 3),
                   {},
                   {9}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG PACK   ", 3),
                   {},
                   {10}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("LG PKG    ", 3),
                   {},
                   {11}}},
                 1, 8);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

// SELECT * FROM lineitem WHERE (l_shipmode = ANY ('{AIR,"AIR REG"}'::bpchar[]))
// AND (l_shipinstruct = 'DELIVER IN PERSON'::bpchar) AND (((l_quantity >=
// '1'::numeric) AND (l_quantity <= '11'::numeric)) OR ((l_quantity >=
// '10'::numeric) AND (l_quantity <= '20'::numeric)) OR ((l_quantity >=
// '20'::numeric) AND (l_quantity <= '30'::numeric)))
void FilterSetup::SetupFilterModuleLineitemQ19(
    modules::FilterInterface& filter_module, const int input_stream_id,
    const int output_stream_id) {
  filter_module.FilterSetStreamIDs(input_stream_id, output_stream_id,
                                   output_stream_id);

  // Possibly don't need to bother with the 1 and 3
  // 0 - AIR REG & >=
  // 1 - AIR REG & >
  // 2 - AIR & >=
  // 3 - AIR & >

  SetOneOutputSingleModuleMode(filter_module);
  // l_quantity
  // Since this module only has 4 compare lanes we will combine the query
  // l_quantity comparisons
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kLessThanOrEqual32Bit,
        {30 * 100},
        {},
        {0, 1, 2, 3}},
       {module_config_values::FilterCompareFunctions::kGreaterThanOrEqual32Bit,
        {1 * 100},
        {module_config_values::LiteralTypes::kLiteralPositive,
         module_config_values::LiteralTypes::kLiteralDontCare,
         module_config_values::LiteralTypes::kLiteralPositive,
         module_config_values::LiteralTypes::kLiteralDontCare},
        {0, 1, 2, 3}}},
      0, 13);
  SetComparisons(
      filter_module,
      {{module_config_values::FilterCompareFunctions::kEqual32Bit,
        {0},
        {},
        {0, 1, 2, 3}},
       {module_config_values::FilterCompareFunctions::kGreaterThan32Bit,
        {1},
        {module_config_values::LiteralTypes::kLiteralDontCare,
         module_config_values::LiteralTypes::kLiteralPositive,
         module_config_values::LiteralTypes::kLiteralDontCare,
         module_config_values::LiteralTypes::kLiteralPositive},
        {0, 1, 2, 3}}},
      0, 14);

  // l_shipmode
  // The table only has REG AIR but we still keep the orignally generated query
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("AIR REG   ", 3),
                   {module_config_values::LiteralTypes::kLiteralPositive,
                    module_config_values::LiteralTypes::kLiteralPositive,
                    module_config_values::LiteralTypes::kLiteralDontCare,
                    module_config_values::LiteralTypes::kLiteralDontCare},
                   {0, 1, 2, 3}},
                  {module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("AIR       ", 3),
                   {module_config_values::LiteralTypes::kLiteralDontCare,
                    module_config_values::LiteralTypes::kLiteralDontCare,
                    module_config_values::LiteralTypes::kLiteralPositive,
                    module_config_values::LiteralTypes::kLiteralPositive},
                   {0, 1, 2, 3}}},
                 1, 8);

  // l_shipinstruct
  SetComparisons(filter_module,
                 {{module_config_values::FilterCompareFunctions::kEqual32Bit,
                   ConvertCharStringToAscii("DELIVER IN PERSON        ", 7),
                   {},
                   {0, 1, 2, 3}}},
                 1, 15);

  filter_module.WriteDNFClauseLiteralsToFilter_4CMP_32DNF(
      query_acceleration_constants::kDatapathWidth);
}

void FilterSetup::SetOneOutputSingleModuleMode(
    modules::FilterInterface& filter_module) {
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

void FilterSetup::SetComparisons(modules::FilterInterface& filter_module,
                                 std::vector<FilterComparison> comparisons,
                                 int chunk_id, int data_position) {
  std::vector<module_config_values::FilterCompareFunctions> compare_functions;
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
        module_config_values::FilterCompareFunctions::kEqual32Bit);
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

FilterSetup::FilterComparison::FilterComparison(
    module_config_values::FilterCompareFunctions compare_function,
    std::vector<int> compare_reference_values,
    const std::vector<module_config_values::LiteralTypes>& literal_types,
    const std::vector<int>& dnf_clause_ids) {
  if (literal_types.empty()) {
    if (dnf_clause_ids.empty()) {
      throw std::runtime_error("No DNF IDs given!");
    }
    std::vector<module_config_values::LiteralTypes> default_literal_types;
    default_literal_types.reserve(dnf_clause_ids.size());
    for (const auto& dnf_id : dnf_clause_ids) {
      default_literal_types.push_back(
          module_config_values::LiteralTypes::kLiteralPositive);
    }
    this->literal_types = default_literal_types;
  } else {
    if (literal_types.size() != dnf_clause_ids.size()) {
      throw std::runtime_error("Incorrect comparison data given!");
    }
    this->literal_types = literal_types;
  }
  this->dnf_clause_ids = dnf_clause_ids;
  this->compare_function = compare_function;
  this->compare_reference_values = std::move(compare_reference_values);
}
