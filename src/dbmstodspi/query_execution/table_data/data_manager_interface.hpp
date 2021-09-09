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

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "memory_block_interface.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::core_interfaces::table_data::TableData;
using orkhestrafs::dbmstodspi::MemoryBlockInterface;

namespace orkhestrafs::dbmstodspi {

class DataManagerInterface {
 public:
  virtual ~DataManagerInterface() = default;

  virtual auto ParseDataFromCSV(
      const std::string& filename,
      const std::vector<ColumnDataType>& column_data_types,
      const std::vector<int>& column_sizes, int& rows_already_read) const
      -> TableData = 0;

  virtual auto WriteDataFromCSVToMemory(
      const std::string& filename,
      const std::vector<std::pair<ColumnDataType, int>>& column_defs_vector,
      const std::unique_ptr<MemoryBlockInterface>& memory_device) const
      -> int = 0;

  virtual auto ReadIntegerDataFromCSV(
      const std::vector<std::pair<ColumnDataType, int>>& table_column_defs,
      const std::string& filename) const -> std::vector<uint32_t> = 0;

  virtual auto GetHeaderColumnVector(
      const std::vector<ColumnDataType>& column_data_types,
      const std::vector<int>& column_sizes) const
      -> std::vector<std::pair<ColumnDataType, int>> = 0;

  virtual void PrintTableData(const TableData& table_data) const = 0;

  virtual void WriteTableData(const TableData& table_data,
                              const std::string& filename) const = 0;
};

}  // namespace orkhestrafs::dbmstodspi