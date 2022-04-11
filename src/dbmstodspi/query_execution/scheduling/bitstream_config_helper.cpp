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

#include "bitstream_config_helper.hpp"

#include <algorithm>

using orkhestrafs::dbmstodspi::BitstreamConfigHelper;

auto BitstreamConfigHelper::GetResultingConfig(
    const std::vector<ScheduledModule>& previous_configuration,
    const std::vector<ScheduledModule>& reduced_current_config,
    const std::vector<ScheduledModule>& reduced_next_config)
    -> std::vector<ScheduledModule> {
  auto left_over_config = reduced_next_config;
  for (const auto& module : previous_configuration) {
    if (std::find(reduced_current_config.begin(), reduced_current_config.end(),
                  module) == reduced_current_config.end()) {
      left_over_config.push_back(module);
    }
  }
  return left_over_config;
}

auto BitstreamConfigHelper::GetConfigCompliments(
    const std::vector<ScheduledModule>& current_run,
    const std::vector<ScheduledModule>& previous_configuration)
    -> std::pair<std::vector<ScheduledModule>, std::vector<ScheduledModule>> {
  auto reduced_next_config = current_run;
  auto reduced_current_config = previous_configuration;

  for (const auto& next_module : current_run) {
    for (const auto& cur_module : previous_configuration) {
      if (cur_module.operation_type == next_module.operation_type &&
          cur_module.bitstream == next_module.bitstream &&
          cur_module.position == next_module.position) {
        reduced_next_config.erase(
            std::remove(reduced_next_config.begin(), reduced_next_config.end(),
                        next_module),
            reduced_next_config.end());
        reduced_current_config.erase(
            std::remove(reduced_current_config.begin(),
                        reduced_current_config.end(), cur_module),
            reduced_current_config.end());
      }
    }
  }

  return {reduced_next_config, reduced_current_config};
}

auto BitstreamConfigHelper::GetOldNonOverlappingModules(
    const std::vector<ScheduledModule>& current_run,
    const std::vector<ScheduledModule>& previous_configuration)
    -> std::vector<ScheduledModule> {
  auto non_overlapping = previous_configuration;
  for (const auto& cur_module : previous_configuration) {
    for (const auto& next_module : current_run) {
      // Assuming inclusive coordinates
      if ((next_module.position.first >= cur_module.position.first &&
           next_module.position.first <= cur_module.position.second) ||
          (next_module.position.second >= cur_module.position.first &&
           next_module.position.second <= cur_module.position.second)) {
        non_overlapping.erase(std::remove(non_overlapping.begin(),
                                          non_overlapping.end(), cur_module),
                              non_overlapping.end());
        break;
      }
    }
  }
  return non_overlapping;
}
