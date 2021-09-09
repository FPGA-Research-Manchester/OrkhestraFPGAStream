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

#include "elastic_module_checker.hpp"

#include <stdexcept>

using orkhestrafs::dbmstodspi::ElasticModuleChecker;

void ElasticModuleChecker::CheckElasticityNeeds(
    const std::vector<StreamDataParameters>& input_stream_parameters,
    QueryOperationType operation_type,
    const std::vector<std::vector<int>>& operation_parameters) {
  if (operation_type == QueryOperationType::kMergeSort &&
      !IsMergeSortBigEnough(input_stream_parameters, operation_parameters)) {
    throw std::runtime_error(
        "Unable to use current merge sort on the given data!");
  }
}

auto ElasticModuleChecker::IsMergeSortBigEnough(
    const std::vector<StreamDataParameters>& input_stream_parameters,
    const std::vector<std::vector<int>>& operation_parameters) -> bool {
  return input_stream_parameters.at(0).stream_record_count <=
         operation_parameters.at(0).at(0) * operation_parameters.at(0).at(1);
}