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

#include "aggregation_sum_setup.hpp"

using easydspi::dbmstodspi::AggregationSumSetup;

void AggregationSumSetup::SetupAggregationSum(
    AggregationSumInterface& aggregation_module, int stream_id,
    const std::vector<std::vector<int>>& operation_parameters) {
  aggregation_module.ResetSumRegisters();
  aggregation_module.DefineInput(stream_id, operation_parameters.at(0).at(0));
  aggregation_module.StartPrefetching(false, false, false);
}
