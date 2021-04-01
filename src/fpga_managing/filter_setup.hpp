#pragma once
#include <vector>

#include <string>

#include "filter_config_values.hpp"
#include "filter_interface.hpp"
class FilterSetup {
 public:
  static void SetupFilterModule(FilterInterface& filter_module, int input_stream_id, int output_stream_id,
      const std::vector<std::vector<int>>& operation_parameters);
 private:

  struct FilterComparison {
    filter_config_values::CompareFunctions compare_function;
    std::vector<int> compare_reference_values;
    std::vector<filter_config_values::LiteralTypes> literal_types;
    std::vector<int> dnf_clause_ids;

    FilterComparison(
        filter_config_values::CompareFunctions compare_function,
        std::vector<int> compare_reference_values,
        const std::vector<filter_config_values::LiteralTypes>& literal_types,
        const std::vector<int>& dnf_clause_ids);
  };

  static void SetOneOutputSingleModuleMode(FilterInterface& filter_module);
  static void SetComparisons(FilterInterface& filter_module,
                             std::vector<FilterComparison> comparisons,
                             int chunk_id, int data_position);

  // Copied from types_converter.hpp
  static auto ConvertCharStringToAscii(const std::string& input_string,
                                       int output_size) -> std::vector<int>;


  // Hard-coded filters for now
  static void SetupFilterModuleCars(FilterInterface& filter_module,
                                    int input_stream_id, int output_stream_id);
  static void SetupFilterModulePartQ19(FilterInterface& filter_module,
                                       int input_stream_id,
                                       int output_stream_id);
  static void SetupFilterModuleLineitemQ19(FilterInterface& filter_module,
                                           int input_stream_id,
                                           int output_stream_id);
  static void SetupFilterModuleFinalQ19(FilterInterface& filter_module,
                                           int input_stream_id,
                                           int output_stream_id);

};
