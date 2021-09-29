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

#include "acceleration_module_setup_interface.hpp"

using orkhestrafs::dbmstodspi::AccelerationModuleSetupInterface;

auto AccelerationModuleSetupInterface::IsMultiChannelStream(bool is_input_stream, int stream_index) -> bool {
  return false;
}

auto AccelerationModuleSetupInterface::GetMultiChannelParams(bool is_input, int stream_index,
                           std::vector<std::vector<int>> operation_parameters) -> std::pair<int, int> {
  return {-1,-1};
}

auto AccelerationModuleSetupInterface::GetStreamRecordSize(
    const StreamDataParameters& stream_parameters) -> int {
  if (stream_parameters.stream_specification.empty()) {
    return stream_parameters.stream_record_size;
  }
  return stream_parameters.stream_specification.size();
}
