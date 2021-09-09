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

#include "config.hpp"

using orkhestrafs::core_interfaces::Config;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Check config validity interface.
 */
class ConfigValueCheckerInterface {
 public:
  virtual ~ConfigValueCheckerInterface() = default;
  /**
   * @brief Check config validity.
   * @param input_config Config to check.
   * @return Boolean flag noting if the input is correct.
   */
  virtual auto Check(Config input_config) -> bool = 0;
};
}  // namespace orkhestrafs::dbmstodspi