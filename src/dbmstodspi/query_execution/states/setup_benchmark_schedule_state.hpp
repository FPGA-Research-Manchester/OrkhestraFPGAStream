﻿/*
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

#include "state_interface.hpp"

using orkhestrafs::dbmstodspi::GraphProcessingFSMInterface;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief State for scheduling which is followed by a debug state.
 */
class SetupBenchmarkScheduleState : public StateInterface {
 public:
  ~SetupBenchmarkScheduleState() override = default;

  auto Execute(GraphProcessingFSMInterface* fsm)
      -> std::unique_ptr<StateInterface> override;
};
}  // namespace orkhestrafs::dbmstodspi