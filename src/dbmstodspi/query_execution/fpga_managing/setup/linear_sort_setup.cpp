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

#include "linear_sort_setup.hpp"

#include "stream_parameter_calculator.hpp"

using orkhestrafs::dbmstodspi::LinearSortSetup;

void LinearSortSetup::SetupLinearSortModule(
    LinearSortInterface& linear_sort_module, int stream_id, int record_size) {
  int chunks_per_record =
      StreamParameterCalculator::CalculateChunksPerRecord(record_size);

  linear_sort_module.SetStreamParams(stream_id, chunks_per_record);

  linear_sort_module.StartPrefetchingData();
}