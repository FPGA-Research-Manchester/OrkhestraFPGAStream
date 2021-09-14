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

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "memory_block_interface.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::dbmstodspi::MemoryBlockInterface;

namespace orkhestrafs::dbmstodspi {

class CSVReaderInterface {
 public:
  virtual ~CSVReaderInterface() = default;

  virtual auto ReadTableData(const std::string& filename, char separator,
                             int& rows_already_read)
      -> std::vector<std::vector<std::string>> = 0;

  virtual auto WriteTableFromFileToMemory(
      const std::string& filename, char separator,
      const std::vector<std::pair<ColumnDataType, int>>& column_defs_vector,
      const std::unique_ptr<MemoryBlockInterface>& memory_device) -> int = 0;
};

}  // namespace orkhestrafs::dbmstodspi