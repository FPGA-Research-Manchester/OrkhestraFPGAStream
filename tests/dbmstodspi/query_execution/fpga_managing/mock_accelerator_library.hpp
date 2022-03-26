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

#include "accelerator_library_interface.hpp"
#include "gmock/gmock.h"

using orkhestrafs::dbmstodspi::AcceleratorLibraryInterface;
using orkhestrafs::dbmstodspi::DMAInterface;
using orkhestrafs::dbmstodspi::DMASetupInterface;
using orkhestrafs::dbmstodspi::ILA;
using orkhestrafs::dbmstodspi::ReadBackModule;

class MockAcceleratorLibrary : public AcceleratorLibraryInterface {
 private:
  using TableMap = std::map<std::string, TableMetadata>;

 public:
  MOCK_METHOD(
      void, SetupOperation,
      (const orkhestrafs::dbmstodspi::AcceleratedQueryNode& node_parameters),
      (override));
  MOCK_METHOD(std::unique_ptr<DMAInterface>, GetDMAModule, (), (override));
  MOCK_METHOD(DMASetupInterface&, GetDMAModuleSetup, (), (override));
  MOCK_METHOD(std::vector<std::unique_ptr<ReadBackModule>>,
              ExportLastModulesIfReadback, (), (override));
  MOCK_METHOD(std::unique_ptr<ILA>, GetILAModule, (), (override));
  MOCK_METHOD(bool, IsMultiChannelStream,
              (bool is_input, int stream_index,
               QueryOperationType operation_type),
              (override));
  MOCK_METHOD((std::pair<int, std::vector<int>>), GetMultiChannelParams,
              (bool is_input, int stream_index,
               QueryOperationType operation_type,
               const std::vector<std::vector<int>>& operation_parameters),
              (override));
  MOCK_METHOD((std::vector<int>), GetNodeCapacity,
              (QueryOperationType operation_type,
               const std::vector<std::vector<int>>& operation_parameters),
              (override));
  MOCK_METHOD((bool), IsNodeConstrainedToFirstInPipeline,
              (QueryOperationType operation_type), (override));
  MOCK_METHOD((bool), IsOperationSorting, (QueryOperationType operation_type),
              (override));
  MOCK_METHOD((std::vector<int>), GetMinSortingRequirements,
              (QueryOperationType operation_type,
               const TableMetadata& table_data),
              (override));
  MOCK_METHOD((TableMap), GetWorstCaseProcessedTables,
              (QueryOperationType operation_type,
               const std::vector<int>& min_capacity,
               const std::vector<std::string>& input_tables,
               const TableMap& data_tables),
              (override));
  MOCK_METHOD((bool), UpdateDataTable,
              (QueryOperationType operation_type,
               const std::vector<int>& module_capacity,
               const std::vector<std::string>& input_table_names,
               const TableMap& data_tables, TableMap& resulting_tables),
              (override));
  MOCK_METHOD((bool), IsInputSupposedToBeSorted,
              (QueryOperationType operation_type), (override));
  MOCK_METHOD((std::vector<std::string>), GetResultingTables,
              (QueryOperationType operation,
               const std::vector<std::string>& table_names,
               const TableMap& tables),
              (override));
  MOCK_METHOD((bool), IsOperationReducingData,
              (QueryOperationType operation_type), (override));
  MOCK_METHOD((bool), IsOperationDataSensitive,
              (QueryOperationType operation_type), (override));
  MOCK_METHOD((orkhestrafs::dbmstodspi::AcceleratedQueryNode),
              GetEmptyModuleNode,
              (QueryOperationType operation, int module_position), (override));
};