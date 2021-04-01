#pragma once

#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"
/**
 * For checking the resource elastic options to better accomodate the input data
 * requirements.
 */
class ElasticModuleChecker {
 public:
  /**
   * Check if the currently selected modules need to be replaced with other
   *  resource elastic variants to better suite the input stream requirements.
   *
   * @param input_stream_parameters Vector of input stream requirements.
   * @param operation_type What operation is going to get used.
   * @param operation_parameters What are the given parameters for the given
   *  operation.
   * @param loaded_modules Which modules have been chosen currently.
   */
  static void CheckElasticityNeeds(
      std::vector<StreamDataParameters> input_stream_parameters,
      operation_types::QueryOperation operation_type,
      std::vector<std::vector<int>> operation_parameters,
      query_scheduling_data::ConfigurableModulesVector loaded_modules);

 private:
  /**
   * Check if the chosen merge sort module is big enough for the input data.
   *
   * @param input_stream_parameters Input data requirements.
   * @param operation_parameters Current operation requirements.
   * @return Is the merge sorter big enough.
   */
  static auto IsMergeSortBigEnough(
      std::vector<StreamDataParameters> input_stream_parameters,
      std::vector<std::vector<int>> operation_parameters) -> bool;
};