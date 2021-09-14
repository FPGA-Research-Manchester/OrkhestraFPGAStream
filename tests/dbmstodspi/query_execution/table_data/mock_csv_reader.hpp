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

#include "csv_reader_interface.hpp"
#include "gmock/gmock.h"

using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::dbmstodspi::CSVReaderInterface;
using orkhestrafs::dbmstodspi::MemoryBlockInterface;

class MockCSVReader : public CSVReaderInterface {
 public:
  virtual auto WriteTableFromFileToMemory(
      const std::string& filename, char separator,
      const ColumDefsVector& column_defs_vector,
      const std::unique_ptr<MemoryBlockInterface>& memory_device) -> int {
    return MockWriteTableFromFileToMemory(
        filename, separator, column_defs_vector, memory_device.get());
  }

  MOCK_METHOD(std::vector<std::vector<std::string>>, ReadTableData,
              (const std::string& filename, char separator,
               int& rows_already_read),
              (override));
  MOCK_METHOD(int, MockWriteTableFromFileToMemory,
              (const std::string& filename, char separator,
               const ColumDefsVector& column_defs_vector,
               const MemoryBlockInterface* memory_device),
              ());
};