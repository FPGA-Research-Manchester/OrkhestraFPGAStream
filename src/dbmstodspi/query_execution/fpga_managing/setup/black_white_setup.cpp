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

#include "black_white_setup.hpp"

#include "logger.hpp"
#include "black_white.hpp"
#include "black_white_interface.hpp"

using orkhestrafs::dbmstodspi::BlackWhiteSetup;

using orkhestrafs::dbmstodspi::BlackWhite;
using orkhestrafs::dbmstodspi::BlackWhiteInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void BlackWhiteSetup::SetupModule(
    AccelerationModule& acceleration_module,
    const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring black_n_white on pos " +
          std::to_string(module_parameters.operation_module_location));
  BlackWhiteSetup::SetupBlackWhiteModule(
      dynamic_cast<BlackWhiteInterface&>(acceleration_module),
      module_parameters.input_streams[0].stream_id);
}

auto BlackWhiteSetup::CreateModule(MemoryManagerInterface* memory_manager,
                                       int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<BlackWhite>(memory_manager, module_postion);
}

void BlackWhiteSetup::SetupBlackWhiteModule(
    BlackWhiteInterface& black_white_module, int stream_id) {
  black_white_module.SetStreamParams(stream_id);
}