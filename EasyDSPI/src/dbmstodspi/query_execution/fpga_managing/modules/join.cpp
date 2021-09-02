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

#include "join.hpp"

#include <cmath>
#include <iostream>

using easydspi::dbmstodspi::Join;

void Join::StartPrefetchingData() { AccelerationModule::WriteToModule(0, 1); }

void Join::DefineOutputStream(int output_stream_chunk_count,
                              int first_input_stream_id,
                              int second_input_stream_id,
                              int output_stream_id) {
  AccelerationModule::WriteToModule(4, ((output_stream_chunk_count - 1) << 24) +
                                           (second_input_stream_id << 16) +
                                           (first_input_stream_id << 8) +
                                           output_stream_id);
}

void Join::SetFirstInputStreamChunkCount(int chunk_count) {
  AccelerationModule::WriteToModule(
      8, static_cast<int>(log2(2 * (chunk_count - 1))));
}

// ceil log
void Join::SetSecondInputStreamChunkCount(int chunk_count) {
  AccelerationModule::WriteToModule(
      12, static_cast<int>(log2(2 * (chunk_count - 1))));
}

void Join::SelectOutputDataElement(int output_chunk_id, int input_chunk_id,
                                   int data_position,
                                   bool is_element_from_second_stream) {
  AccelerationModule::WriteToModule(
      (1 << 13) + (output_chunk_id << 7) + (data_position << 2),
      (static_cast<int>(is_element_from_second_stream) << 16) + input_chunk_id);
}

void Join::Reset() { AccelerationModule::WriteToModule(16, 1); }