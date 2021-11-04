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

#include "pre_scheduling_processor.hpp"

using orkhestrafs::dbmstodspi::PreSchedulingProcessor;

auto PreSchedulingProcessor::GetMinimumCapacityValuesFromHWLibrary(
    const std::map<QueryOperationType, OperationPRModules>& hw_library)
    -> std::map<QueryOperationType, std::vector<int>> {
  std::map<QueryOperationType, std::vector<int>> min_capacity_map;
  for ( const auto &[operation, operation_bitstreams_data]: hw_library ) {
    for ( const auto &[bitstream_name, bitstream_parameters]: operation_bitstreams_data.bitstream_map) {
      const auto &[operation_iterator, inserted_flag] = min_capacity_map.try_emplace(operation, bitstream_parameters.capacity);
      if (!inserted_flag) {
        for (int capacity_param_index = 0; capacity_param_index < bitstream_parameters.capacity.size(); capacity_param_index++){
          if (operation_iterator->second.at(capacity_param_index) > bitstream_parameters.capacity.at(capacity_param_index)) {
            operation_iterator->second[capacity_param_index] = bitstream_parameters.capacity.at(capacity_param_index);
          }
        }
      }
    }
  }
  return min_capacity_map;
}