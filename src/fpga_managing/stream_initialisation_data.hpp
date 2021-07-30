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
#include <memory>
#include <string>

#include "memory_block_interface.hpp"

namespace dbmstodspi::fpga_managing {

/**
 * @brief Struct to hold data from which the configuration data can be
 * calculated before FPGA configuration
 */
struct StreamInitialisationData {
  /// ID
  int stream_id;
  /// Data CSV file location.
  std::string stream_data_file_name;
  /// Memory mapped DDR location where the data will be written to.
  std::unique_ptr<MemoryBlockInterface> memory_block;
};

}  // namespace dbmstodspi::fpga_managing