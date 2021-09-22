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

#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"

namespace orkhestrafs::dbmstodspi {

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
   * @return Boolean flag noting if the run is valid.
   */
  static auto IsRunValid(
      const std::vector<StreamDataParameters>& input_stream_parameters,
      QueryOperationType operation_type,
      const std::vector<std::vector<int>>& operation_parameters) -> bool;

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
      const std::vector<StreamDataParameters>& input_stream_parameters,
      const std::vector<std::vector<int>>& operation_parameters) -> bool;

  static auto IsMergeSortConfiguredCorrectly(
      const std::vector<StreamDataParameters>& input_stream_parameters,
      const std::vector<std::vector<int>>& operation_parameters) -> bool;

  static auto IsMergeSortWithOneStream(
      const std::vector<StreamDataParameters>& input_stream_parameters,
      const std::vector<std::vector<int>>& operation_parameters) -> bool;
};
}  // namespace orkhestrafs::dbmstodspi