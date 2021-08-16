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

#include "input_manager.hpp"

#include <memory>

using easydspi::core::core_input::InputManager;

std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
InputManager::parse(std::string input_filename, std::string config_filename) {
  std::string configurations_library = "CONFIGURATIONS_LIBRARY";
  std::string driver_selection = "DRIVER_SELECTION";
  std::string memory_requirements = "BITSTREAMS_MEM_REQ";
  std::string data_type_sizes = "DATA_SIZE_CONFIG";

  auto json_file_names =
      input_config_reader_->ParseInputConfig(config_filename);

  // Just for idea demonstration
  if (json_validator_) {
    for (const auto& [key, value] : json_file_names) {
      json_validator_->check(value);
    }
    json_validator_->check(input_filename);
  }

  Config config;
  config.accelerator_library = json_reader_->readAcceleratorLibrary(
      json_file_names[configurations_library]);
  config.data_sizes =
      json_reader_->readDataSizes(json_file_names[data_type_sizes]);
  config.driver_library =
      json_reader_->readDriverLibrary(json_file_names[driver_selection]);
  config.required_memory_space =
      json_reader_->readReqMemorySpace(json_file_names[memory_requirements]);

  if (config_validator_) {
    config_validator_->check(config);
  }

  auto graph = graph_creator_->makeGraph(input_filename);

  return std::make_pair(std::move(graph), config);
}