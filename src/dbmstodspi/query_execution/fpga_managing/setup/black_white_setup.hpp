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

#pragma once
#include <vector>

#include "acceleration_module_setup_interface.hpp"
#include "black_white_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to calculate the module setup to convert an image to black and white image.
 */
class BlackWhiteSetup : public AccelerationModuleSetupInterface {
 public:
  void SetupModule(AccelerationModule& acceleration_module,
                   const AcceleratedQueryNode& module_parameters) override;
  auto CreateModule(MemoryManagerInterface* memory_manager, int module_postion)
      -> std::unique_ptr<AccelerationModule> override;

 private:
  static void SetupBlackWhiteModule(BlackWhiteInterface& black_white_module,
                                    int stream_id);
};

}  // namespace orkhestrafs::dbmstodspi