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

using orkhestrafs::core::core_input::InputManager;
using orkhestrafs::core_interfaces::Config;

auto InputManager::Parse(std::string input_filename,
                         std::string config_filename)
    -> std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config> {
  auto config = config_creator_.GetConfig(config_filename);
  auto graph = graph_creator_->MakeGraph(input_filename);

  return std::make_pair(std::move(graph), config);
}