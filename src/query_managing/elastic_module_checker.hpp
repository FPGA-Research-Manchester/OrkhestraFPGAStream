#pragma once

#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"

namespace dbmstodspi::query_managing {

/**
 * @brief For checking the resource elastic options to better accomodate the
 * input data requirements.
 */
class ElasticModuleChecker {
 public:
  /**
   * @brief Check if the currently selected modules need to be replaced with
   * other resource elastic variants to better suite the input stream
   * requirements.
   *
   * @param input_stream_parameters Vector of input stream requirements.
   * @param operation_type What operation is going to get used.
   * @param operation_parameters What are the given parameters for the given
   * operation.
   */
  static void CheckElasticityNeeds(
      const std::vector<fpga_managing::StreamDataParameters>&
          input_stream_parameters,
      fpga_managing::operation_types::QueryOperationType operation_type,
      const std::vector<std::vector<int>>& operation_parameters);

 private:
  /**
   * @brief Check if the chosen merge sort module is big enough for the input
   * data.
   *
   * @param input_stream_parameters Input data requirements.
   * @param operation_parameters Current operation requirements.
   * @return Is the merge sorter big enough.
   */
  static auto IsMergeSortBigEnough(
      const std::vector<fpga_managing::StreamDataParameters>&
          input_stream_parameters,
      const std::vector<std::vector<int>>& operation_parameters) -> bool;
};

}  // namespace dbmstodspi::query_managing