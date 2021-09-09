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

#include <string>

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Interface describing classes able to check JSON contents.
 */
class JSONValidatorInterface {
 public:
  virtual ~JSONValidatorInterface() = default;
  /**
   * @brief Check json format.
   * @param json_filename File to be checked.
   * @return Boolean flag noting the validity of the JSON file.
   */
  virtual auto Check(std::string json_filename) -> bool = 0;
};
}  // namespace orkhestrafs::dbmstodspi