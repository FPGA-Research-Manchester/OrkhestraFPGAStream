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

#include "aggregation_sum.hpp"

using orkhestrafs::dbmstodspi::AggregationSum;

void AggregationSum::StartPrefetching(bool request_data, bool destroy_data,
                                      bool count_data) {
  AccelerationModule::WriteToModule(0,
                                    (static_cast<int>(request_data) << 2) +
                                        (static_cast<int>(destroy_data) << 1) +
                                        static_cast<int>(count_data));
}

void AggregationSum::DefineInput(int stream_id, int chunk_id) {
  AccelerationModule::WriteToModule(4, (chunk_id << 8) + stream_id);
}

auto AggregationSum::ReadSum(int data_position, bool is_low) -> uint32_t {
  return AccelerationModule::ReadFromModule(64 + data_position * 8 +
                                            (static_cast<int>(!is_low) * 4));
}

auto AggregationSum::ReadResult(int data_position) -> uint32_t {
  return AggregationSum::ReadSum(data_position / 2, data_position % 2 == 0);
}

auto AggregationSum::IsModuleActive() -> bool {
  return (1 & AccelerationModule::ReadFromModule(4)) != 0U;
}

void AggregationSum::ResetSumRegisters() {
  AccelerationModule::WriteToModule(64, 1);
}
