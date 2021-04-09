#pragma once
#include <string>
#include <vector>

#include "filter_config_values.hpp"
#include "filter_interface.hpp"

namespace dbmstodspi {
namespace fpga_managing {

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
   * Currently operation parameters are used to select between hardcoded filter
   * configurations.
   * @param filter_module Filter instance which is used to write to memory
   * mapped registers.
   * @param input_stream_id Input stream ID.
   * @param output_stream_id Output stream ID.
   * @param operation_parameters Operation parameters to setup the filter with.
   */
  static void SetupFilterModule(
      FilterInterface& filter_module, int input_stream_id, int output_stream_id,
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
  static void SetupFilterModuleFinalDoubleQ19(FilterInterface& filter_module,
                                              int input_stream_id,
                                              int output_stream_id);
};

}  // namespace fpga_managing
}  // namespace dbmstodspi
