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

#include "interactive_state.hpp"

#include <iostream>

#include "logger.hpp"
#include "schedule_state.hpp"

using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;
using orkhestrafs::dbmstodspi::ScheduleState;
using orkhestrafs::dbmstodspi::InteractiveState;
using orkhestrafs::dbmstodspi::StateInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

auto InteractiveState::Execute(GraphProcessingFSMInterface* fsm)
    -> std::unique_ptr<StateInterface> {
  Log(LogLevel::kTrace, "In interactive state");

  std::cout << "What operation do you want to do?" << std::endl;
  std::string current_input = "";
  std::cin >> current_input;
  if (current_input == "hello;") {
    std::cout << "Hello world!" << std::endl;
  }
  std::cout << "You wrote:" << std::endl;
  std::cout << current_input << std::endl;

  return std::make_unique<InteractiveState>();
}