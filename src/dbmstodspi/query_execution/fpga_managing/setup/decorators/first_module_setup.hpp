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

#include "acceleration_module_setup_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class for noting which module has to get it's input form the DMA.
 */
class FirstModuleSetup : public virtual AccelerationModuleSetupInterface {
 public:
  auto IsConstrainedToFirstInPipeline() -> bool override;
};
}  // namespace orkhestrafs::dbmstodspi
