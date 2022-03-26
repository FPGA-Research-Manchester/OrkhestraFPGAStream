/*
Copyright 2022 University of Manchester

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
#include <utility>
#include <vector>

#include "scheduled_module.hpp"

using orkhestrafs::dbmstodspi::ScheduledModule;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class containing helper methods for configuration differences
 * handling.
 */
class BitstreamConfigHelper {
 public:
  static auto GetResultingConfig(
      const std::vector<ScheduledModule>& previous_configuration,
      const std::vector<ScheduledModule>& reduced_current_config,
      const std::vector<ScheduledModule>& reduced_next_config)
      -> std::vector<ScheduledModule>;

  static auto GetConfigCompliments(
      const std::vector<ScheduledModule>& current_run,
      const std::vector<ScheduledModule>& previous_configuration)
      -> std::pair<std::vector<ScheduledModule>, std::vector<ScheduledModule>>;

  static auto GetOldNonOverlappingModules(
      const std::vector<ScheduledModule>& current_run,
      const std::vector<ScheduledModule>& previous_configuration)
      -> std::vector<ScheduledModule>;
};

}  // namespace orkhestrafs::dbmstodspi