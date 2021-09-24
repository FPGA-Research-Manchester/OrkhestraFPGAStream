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
#include <vector>

#include "dma_interface.hpp"
#include "stream_data_parameters.hpp"
#include "memory_manager_interface.hpp"

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Interface for classes which can setup DMA modules.
 */
class DMASetupInterface {
 public:
  virtual ~DMASetupInterface() = default;
  virtual void SetupDMAModule(
      DMAInterface& dma_module,
      const std::vector<StreamDataParameters>& input_streams,
      const std::vector<StreamDataParameters>& output_streams) = 0;
  virtual auto CreateDMAModule(MemoryManagerInterface* memory_manager)
      -> std::unique_ptr<DMAInterface> = 0;
};

}  // namespace orkhestrafs::dbmstodspi