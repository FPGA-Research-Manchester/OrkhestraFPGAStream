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

namespace orkhestrafs::core_interfaces {
/**
 * @brief Interface to class to describe a class parsing config and inpud graph.
 */
class InputManagerInterface {
 public:
  virtual ~InputManagerInterface() = default;
  /**
   * @brief Parse the files to create an execution graph and a config.
   * @param input_filename Operation graph definition.
   * @param config_filename Operator library configuration files.
   * @return The execution graph and a config configuring the operator library.
   */
  virtual std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config> Parse(
      std::string input_filename, std::string config_filename) = 0;
};
}  // namespace orkhestrafs::core_interfaces