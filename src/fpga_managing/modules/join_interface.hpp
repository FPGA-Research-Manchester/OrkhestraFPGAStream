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

namespace dbmstodspi::fpga_managing::modules {

/**
 * @brief Interface class to be implemented by #Join
 */
class JoinInterface {
 public:
  virtual ~JoinInterface() = default;

  virtual void DefineOutputStream(int output_stream_chunk_count,
                                  int first_input_stream_id,
                                  int second_input_stream_id,
                                  int output_stream_id) = 0;
  virtual void SetFirstInputStreamChunkCount(int chunk_count) = 0;
  virtual void SetSecondInputStreamChunkCount(int chunk_count) = 0;
  virtual void SelectOutputDataElement(int output_chunk_id, int input_chunk_id,
                                       int data_position,
                                       bool is_element_from_second_stream) = 0;
  virtual void StartPrefetchingData() = 0;
  virtual void Reset() = 0;
};

}  // namespace dbmstodspi::fpga_managing::modules