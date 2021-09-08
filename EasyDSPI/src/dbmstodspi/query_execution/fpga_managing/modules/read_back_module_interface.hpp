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
 * @brief Interface class for modules which need to get it's result read from
 * the configuration registers.
 */
class ReadBackModuleInterface {
 public:
  virtual ~ReadBackModuleInterface() = default;

  /**
   * @brief Read the result of the module.
   * @param data_position Bus index of the interface to read from.
   * @return 32 bit value written in the result register of the module.
  */
  virtual auto ReadResult(int data_position) -> uint32_t = 0;
};

}  // namespace easydspi::dbmstodspi