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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.hpp"
#include "execution_plan_graph_interface.hpp"

namespace easydspi::core_interfaces {
class InputManagerInterface {
 public:
  virtual ~InputManagerInterface() = default;
  virtual std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config> parse(
      std::string input_filename, std::string config_filename) = 0;
};
}  // namespace easydspi::core_interfaces