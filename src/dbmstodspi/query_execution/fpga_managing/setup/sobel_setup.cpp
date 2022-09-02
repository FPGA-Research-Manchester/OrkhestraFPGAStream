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

#include "sobel_setup.hpp"

#include "logger.hpp"
#include "sobel.hpp"
#include "sobel_interface.hpp"

using orkhestrafs::dbmstodspi::SobelSetup;

using orkhestrafs::dbmstodspi::Sobel;
using orkhestrafs::dbmstodspi::SobelInterface;
using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;

void SobelSetup::SetupModule(AccelerationModule& acceleration_module,
                             const AcceleratedQueryNode& module_parameters) {
  Log(LogLevel::kInfo,
      "Configuring sobel on pos " +
          std::to_string(module_parameters.operation_module_location));
  SobelSetup::SetupSobelModule(
      dynamic_cast<SobelInterface&>(acceleration_module),
      module_parameters.input_streams[0].stream_id);
}

auto SobelSetup::CreateModule(MemoryManagerInterface* memory_manager,
                              int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<Sobel>(memory_manager, module_postion);
}

void SobelSetup::SetupSobelModule(SobelInterface& sobel_module, int stream_id) {
  // TODO: hardcoded passthrough!
  sobel_module.SetStreamParams(stream_id, 0, 0);
}