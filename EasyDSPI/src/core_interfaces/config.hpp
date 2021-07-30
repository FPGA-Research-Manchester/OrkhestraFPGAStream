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
#include <utility>
#include <vector>

/// Can be change to have a dictionary of config file names to make it more
/// generic

namespace easydspi::core_interfaces {
struct Config {
  // Constructed run time for now. So doesn't even really need to be here.
  std::map<std::string, std::vector<std::vector<int>>> module_library;

  std::map<std::vector<std::pair<std::string, std::vector<int>>>, std::string>
      accelerator_library;

  std::map<std::string, int> required_memory_space;
  // Maybe int?
  std::map<std::string, double> data_sizes;
  std::map<std::string, std::string> driver_library;
};
}  // namespace easydspi::core_interfaces