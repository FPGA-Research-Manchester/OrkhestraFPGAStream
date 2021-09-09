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

#include "input_manager_factory.hpp"

#include <stdexcept>

#include "config_creator.hpp"
#include "graph_creator.hpp"
#include "input_config_reader.hpp"
#include "input_manager.hpp"
#include "rapidjson_reader.hpp"
#include "graph_creator_interface.hpp"

using orkhestrafs::core::core_input::ConfigCreator;
using orkhestrafs::core::core_input::InputManager;
using orkhestrafs::core::core_input::InputManagerFactory;
using orkhestrafs::core_interfaces::InputManagerInterface;
using orkhestrafs::dbmstodspi::GraphCreator;
using orkhestrafs::dbmstodspi::InputConfigReader;
using orkhestrafs::dbmstodspi::RapidJSONReader;

std::unique_ptr<InputManagerInterface> InputManagerFactory::GetManager() {
  return std::make_unique<InputManager>(
      std::make_unique<GraphCreator>(std::make_unique<RapidJSONReader>(),
                                     nullptr),
      ConfigCreator(std::make_unique<RapidJSONReader>(),
                    std::make_unique<InputConfigReader>(), nullptr, nullptr));
}