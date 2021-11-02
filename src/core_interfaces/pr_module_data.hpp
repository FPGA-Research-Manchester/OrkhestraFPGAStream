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

#include <map>
#include <string>
#include <vector>

namespace orkhestrafs::core_interfaces::hw_library {

/**
 * @brief Data about a single PR module on where it fits on the board
 */
struct PRModuleData {
  std::vector<int> fitting_locations;
  int length;
  std::vector<int> capacity;
  std::string resource_string;
  bool is_backwards;
};

/**
 * @brief Data containing all PR modules for a certain operation and a vector
 * placing all of the modules.
 */
struct OperationPRModules {
  std::vector<std::vector<std::string>> starting_locations;
  std::map<std::string, PRModuleData> bitstream_map;
};

}  // namespace orkhestrafs::core_interfaces::hw_library