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

#include "config.hpp"
#include "execution_manager_interface.hpp"

using orkhestrafs::core_interfaces::Config;
using orkhestrafs::core_interfaces::ExecutionManagerInterface;

namespace orkhestrafs::core::core_execution {
/**
 * @brief Class to create execution managers.
 */
class ExecutionManagerFactory {
 public:
  /**
   * @brief Create a manager described by the execution manager interface.
   * @param config Config to setup the operators library
   * @return A smart pointer to the execution manager class to execute the FSM
   * with.
   */
  static auto GetManager(const Config& config, bool is_interactive)
      -> std::unique_ptr<ExecutionManagerInterface>;
};
}  // namespace orkhestrafs::core::core_execution