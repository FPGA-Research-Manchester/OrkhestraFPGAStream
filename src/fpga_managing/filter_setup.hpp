#pragma once
#include <vector>

#include <string>

#include "filter_config_values.hpp"
#include "filter_interface.hpp"
class FilterSetup {
 public:
  static void SetupFilterModule1(FilterInterface& filter_module,
                                 int input_stream_id, int output_stream_id);
  static void SetupFilterModule2(FilterInterface& filter_module,
                                 int input_stream_id, int output_stream_id);
  static void SetupFilterModule3(FilterInterface& filter_module,
                                 int input_stream_id, int output_stream_id);

 private:
  struct FilterComparison {
    filter_config_values::CompareFunctions compare_function;
    std::vector<int> compare_reference_values;
    std::vector<filter_config_values::LiteralTypes> literal_types;
    std::vector<int> dnf_clause_ids;
  };

  static void SetOneOutputSingleModuleMode(FilterInterface& filter_module);
  static void SetComparisons(FilterInterface& filter_module,
                             std::vector<FilterComparison> comparisons,
                             int chunk_id, int data_position);

  // Copied from types_converter.hpp
  static auto ConvertCharStringToAscii(const std::string& input_string,
                                       int output_size) -> std::vector<int>;
};
