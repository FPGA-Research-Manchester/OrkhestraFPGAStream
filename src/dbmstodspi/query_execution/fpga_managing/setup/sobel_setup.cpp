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
  if (module_parameters.operation_parameters.empty() ||
      module_parameters.operation_parameters.front().size() != 2) {
    throw std::runtime_error("Image parameters are undefined!");
  }
  if (module_parameters.input_streams[0].stream_id != 15) {
    SobelSetup::SetupSobelModule(
        dynamic_cast<SobelInterface&>(acceleration_module),
        module_parameters.input_streams[0].stream_id,
        module_parameters.operation_parameters.front()
            .front(),
        module_parameters.operation_parameters.front().back());
  } else {
    SobelSetup::SetupPassthroughSobelModule(
        dynamic_cast<SobelInterface&>(acceleration_module),
        module_parameters.input_streams[0].stream_id);
  }
}

auto SobelSetup::CreateModule(MemoryManagerInterface* memory_manager,
                              int module_postion)
    -> std::unique_ptr<AccelerationModule> {
  return std::make_unique<Sobel>(memory_manager, module_postion);
}

void SobelSetup::SetupSobelModule(SobelInterface& sobel_module, int stream_id,
                                  int image_width, int image_height) {
  // Each record is 64 pixels
  if (image_width % 64 != 0 || image_width == 64) {
    throw std::runtime_error("Unsopported image dimensions!");
  }
  sobel_module.SetStreamParams(stream_id, image_width / 64,
                               image_height);
}

void SobelSetup::SetupPassthroughSobelModule(SobelInterface& sobel_module,
                                             int stream_id) {
  sobel_module.SetStreamParams(stream_id, 0, 0);
}