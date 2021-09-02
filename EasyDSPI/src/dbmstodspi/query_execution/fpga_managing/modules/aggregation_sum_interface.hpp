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

#include <cstdint>

namespace easydspi::dbmstodspi {

/**
 * @brief Interface class which is implemented in the #AggregationSum class.
 */
class AggregationSumInterface {
 public:
  virtual ~AggregationSumInterface() = default;

  virtual void StartPrefetching(bool request_data, bool destroy_data,
                                bool count_data) = 0;
  virtual void DefineInput(int stream_id, int chunk_id) = 0;
  virtual auto ReadSum(int data_position, bool is_low) -> uint32_t = 0;
  virtual auto IsModuleActive() -> bool = 0;
  virtual void ResetSumRegisters() = 0;
};

}  // namespace easydspi::dbmstodspi