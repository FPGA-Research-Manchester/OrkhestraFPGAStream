#pragma once
#include <string>
#include <vector>

#include "filter_interface.hpp"

namespace dbmstodspi::fpga_managing {

/**
 * @brief Filter setup class which calculates the correct configuration data to
 * be written into the filter configuration registers.
 */
class FilterSetup {
 public:
  /**
   * @brief Setup filter module according to the input and output streams and
   * the given operation parameters.
   *
   * For the operation parameters the following formatting is assumed:
   * The first vector contains the chunk_id, position_id, comparison_count
   * 
   * For each comparison there are 4 vectors:
   * Compare function
   * Reference values
   * Literal types
   * DNF clause IDs
   * 
   * @param filter_module Filter instance which is used to write to memory
   * mapped registers.
   * @param input_stream_id Input stream ID.
   * @param output_stream_id Output stream ID.
   * @param operation_parameters Operation parameters to setup the filter with.
   */
  static void SetupFilterModule(
      modules::FilterInterface& filter_module, int input_stream_id,
      int output_stream_id,
      const std::vector<std::vector<int>>& operation_parameters);

 private:
  struct FilterComparison {
    module_config_values::FilterCompareFunctions compare_function;
    std::vector<int> compare_reference_values;
    std::vector<module_config_values::LiteralTypes> literal_types;
    std::vector<int> dnf_clause_ids;

    FilterComparison(
        module_config_values::FilterCompareFunctions compare_function,
        std::vector<int> compare_reference_values,
        const std::vector<module_config_values::LiteralTypes>& literal_types,
        const std::vector<int>& dnf_clause_ids);
  };

  static void SetOneOutputSingleModuleMode(
      modules::FilterInterface& filter_module);
  static void SetComparisons(modules::FilterInterface& filter_module,
                             std::vector<FilterComparison> comparisons,
                             int chunk_id, int data_position);

  static void SetAllComparisons(
      modules::FilterInterface& filter_module,
      const std::vector<std::vector<int>>& operation_parameters);

  // Don't need any more
  // Copied from types_converter.hpp
  static auto ConvertCharStringToAscii(const std::string& input_string,
                                       int output_size) -> std::vector<int>;

  // Hard-coded filters for now
  static void SetupFilterModuleCars(modules::FilterInterface& filter_module,
                                    int input_stream_id, int output_stream_id);
  static void SetupFilterModulePartQ19(modules::FilterInterface& filter_module,
                                       int input_stream_id,
                                       int output_stream_id);
  static void SetupFilterModuleLineitemQ19(
      modules::FilterInterface& filter_module,
                                           int input_stream_id,
                                           int output_stream_id);
  static void SetupFilterModuleFinalQ19(modules::FilterInterface& filter_module,
                                        int input_stream_id,
                                        int output_stream_id);
  static void SetupFilterModuleFinalDoubleQ19(
      modules::FilterInterface& filter_module,
                                              int input_stream_id,
                                              int output_stream_id);
};

}  // namespace dbmstodspi::fpga_managing
