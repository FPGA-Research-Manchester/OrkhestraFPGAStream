/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "filter_setup.hpp"

#include <cstdio>
#include <iostream>
#include <utility>

#include "filter.hpp"
#include "filter_interface.hpp"
#include "logger.hpp"
#include "query_acceleration_constants.hpp"

using orkhestrafs::dbmstodspi::Filter;
using orkhestrafs::dbmstodspi::FilterInterface;
using orkhestrafs::dbmstodspi::FilterSetup;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void FilterSetup::SetupModule(AccelerationModule& acceleration_module,
                              const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring filter on pos " +
          std::to_string(module_parameters.operation_module_location));
  FilterSetup::SetupFilterModule(
      dynamic_cast<FilterInterface&>(acceleration_module),
      module_parameters.input_streams[0].stream_id,
      module_parameters.output_streams[0].stream_id,
      module_parameters.operation_parameters);
}

auto FilterSetup::CreateModule(MemoryManagerInterface* memory_manager,
                               int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<Filter>(memory_manager, module_postion);
}

void FilterSetup::SetupFilterModule(
    FilterInterface& filter_module, int input_stream_id, int output_stream_id,
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
}

void FilterSetup::SetAllComparisons(
    FilterInterface& filter_module,
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
    for (int i = 0; i < comparison_count; i++) {
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
      current_vector_id += number_of_parameter_vectors;
    }
    SetComparisons(filter_module, current_comparison_parameters,
                   current_chunk_id, current_data_position);
    current_vector_id++;
  }
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
