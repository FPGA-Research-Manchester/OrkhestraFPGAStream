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

#include "mock_acceleration_module.hpp"

#include <cstdint>

using orkhestrafs::dbmstodspi::AccelerationModule;

MockAccelerationModule::~MockAccelerationModule() = default;

void MockAccelerationModule::WriteToModule(int module_internal_address,
                                           uint32_t write_data) {
  AccelerationModule::WriteToModule(module_internal_address, write_data);
}

auto MockAccelerationModule::ReadFromModule(int module_internal_address)
    -> uint32_t {
  return AccelerationModule::ReadFromModule(module_internal_address);
}
