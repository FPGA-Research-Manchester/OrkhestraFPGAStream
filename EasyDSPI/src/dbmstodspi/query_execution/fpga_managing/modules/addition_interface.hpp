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

#include <array>
#include <bitset>
#include <cstdint>
#include <utility>

namespace easydspi::dbmstodspi {

/**
 * @brief Interface class which is implemented in the #Addition class.
 */
class AdditionInterface {
 public:
  virtual ~AdditionInterface() = default;

  virtual void DefineInput(int stream_id, int chunk_id) = 0;
  virtual void SetInputSigns(std::bitset<8> is_value_negative) = 0;
  virtual void SetLiteralValues(
      std::array<std::pair<uint32_t, uint32_t>, 8> literal_values) = 0;
};

}  // namespace easydspi::dbmstodspi