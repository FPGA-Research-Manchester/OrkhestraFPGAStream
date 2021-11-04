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

#pragma once
#include <string>
#include <vector>

#include "acceleration_module_setup_interface.hpp"
#include "filter_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Filter setup class which calculates the correct configuration data to
 * be written into the filter configuration registers.
 */
class FilterSetup : public AccelerationModuleSetupInterface {
 public:
  void SetupModule(AccelerationModule& acceleration_module,
                   const AcceleratedQueryNode& module_parameters) override;
  auto CreateModule(MemoryManagerInterface* memory_manager, int module_position)
      -> std::unique_ptr<AccelerationModule> override;
  auto GetCapacityRequirement(
      std::vector<std::vector<int>> operation_parameters)
      -> std::vector<int> override;

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
      FilterInterface& filter_module, int input_stream_id, int output_stream_id,
      const std::vector<std::vector<int>>& operation_parameters);

 private:

  static const int kChunkIdIndex = 0;
  static const int kDataPositionIndex = 1;
  static const int kComparisonCountIndex = 2;
  static const int kNumberOfParameterVectors = 4;

  static const int kCompareFunctionOffset = 1;
  static const int kCompareRefgerenceValuesOffset = 2;
  static const int kLiteralTypeOffset = 3;
  static const int kDNFClauseIdsOffset = 4;

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

  static void SetOneOutputSingleModuleMode(FilterInterface& filter_module);
  static void SetComparisons(FilterInterface& filter_module,
                             std::vector<FilterComparison> comparisons,
                             int chunk_id, int data_position);

  static void SetAllComparisons(
      FilterInterface& filter_module,
      const std::vector<std::vector<int>>& operation_parameters);
};
}  // namespace orkhestrafs::dbmstodspi
