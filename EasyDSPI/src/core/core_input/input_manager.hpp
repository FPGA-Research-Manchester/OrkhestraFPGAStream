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

#include "config_creator.hpp"
#include "graph_creator_interface.hpp"
#include "input_manager_interface.hpp"

using easydspi::core_interfaces::InputManagerInterface;
using easydspi::dbmstodspi::GraphCreatorInterface;

namespace easydspi::core::core_input {

class InputManager : public InputManagerInterface {
 private:
  std::unique_ptr<GraphCreatorInterface> graph_creator_;
  ConfigCreator config_creator_;

 public:
  ~InputManager() override = default;
  InputManager(std::unique_ptr<GraphCreatorInterface> graph_creator,
               ConfigCreator config_creator)
      : graph_creator_{std::move(graph_creator)},
        config_creator_{std::move(config_creator)} {};

  std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config> Parse(
      std::string input_filename, std::string config_filename) override;
};
}  // namespace easydspi::core::core_input