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

#include "data_manager_interface.hpp"
#include "gmock/gmock.h"

using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::core_interfaces::table_data::TableData;
using orkhestrafs::dbmstodspi::DataManagerInterface;

class MockDataManager : public DataManagerInterface {
 private:
  using ColumDefsVector = std::vector<std::pair<ColumnDataType, int>>;

 public:
  MOCK_METHOD(TableData, ParseDataFromCSV,
              (const std::string& filename,
               const std::vector<ColumnDataType>& column_data_types,
               const std::vector<int>& column_sizes, int& rows_already_read),
              (const, override));
  MOCK_METHOD(int, WriteDataFromCSVToMemory,
              (const std::string& filename,
               const ColumDefsVector& column_defs_vector,
               MemoryBlockInterface* memory_device),
              (const));
  MOCK_METHOD(ColumDefsVector, GetHeaderColumnVector,
              (const std::vector<ColumnDataType>& column_data_types,
               const std::vector<int>& column_sizes),
              (const, override));
  MOCK_METHOD(void, PrintTableData, (const TableData& table_data),
              (const, override));
  MOCK_METHOD(void, WriteTableData,
              (const TableData& table_data, const std::string& filename),
              (const, override));
  MOCK_METHOD(void, WriteRawTableData,
              (const TableData& table_data, const std::string& filename),
              (const, override));
};