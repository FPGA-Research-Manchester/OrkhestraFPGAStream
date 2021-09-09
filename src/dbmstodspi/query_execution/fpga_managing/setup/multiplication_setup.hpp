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
#include <vector>

#include "multiplication_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to calculate the Multiplication module setup.
 */
class MultiplicationSetup {
 private:
 public:
  /**
   * @brief Method to setup the multiplication operation acceleration.
   *
   * The operation parameters will consist of multiple vectors where the first
   * integer notes which chunk the following selection data is configuring.
   *
   * @param addition_module Instance of the accelerator to have access to the
   * memory mapped configuration space.
   * @param active_stream_ids Which streams this module should operate on?
   * @param operation_parameters The operation parameters to configure this
   * module.
   */
  static void SetupMultiplicationModule(
      MultiplicationInterface& multiplication_module,
      const std::vector<int>& active_stream_ids,
      const std::vector<std::vector<int>>& operation_parameters);
};

}  // namespace orkhestrafs::dbmstodspi