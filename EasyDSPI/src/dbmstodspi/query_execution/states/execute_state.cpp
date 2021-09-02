﻿/*
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

#include "execute_state.hpp"

#include <iostream>

using easydspi::dbmstodspi::ExecuteState;
using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::StateInterface;

std::unique_ptr<StateInterface> ExecuteState::execute(
    GraphProcessingFSMInterface* fsm) {
  // Read some data or do something
  std::cout << "Execute" << std::endl;
  fsm->setFinishedFlag();

  return std::make_unique<ExecuteState>();
}