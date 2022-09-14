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
#include "acceleration_module.hpp"
#include "memory_manager_interface.hpp"
#include "sobel_interface.hpp"

namespace orkhestrafs::dbmstodspi {

class Sobel : public AccelerationModule,
                       public SobelInterface {
 private:
 public:
  ~Sobel() override = default;

  explicit Sobel(MemoryManagerInterface* memory_manager,
                          int module_position)
      : AccelerationModule(memory_manager, module_position){};

  void SetStreamParams(int stream_id, int chunks_per_row,
                       int row_count) override;
};

}  // namespace orkhestrafs::dbmstodspi