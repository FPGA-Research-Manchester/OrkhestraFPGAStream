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
#include <utility>
#include <vector>

#include "accelerated_query_node.hpp"
#include "dma_interface.hpp"
#include "dma_setup_interface.hpp"
#include "ila.hpp"
#include "operation_types.hpp"
#include "read_back_module.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::table_data::TableMetadata;

namespace orkhestrafs::dbmstodspi {
/**
 * @brief Implemented by #AcceleratorLibrary
 */
class AcceleratorLibraryInterface {
 public:
  virtual ~AcceleratorLibraryInterface() = default;
  virtual void SetupOperation(const AcceleratedQueryNode& node_parameters) = 0;
  virtual auto GetDMAModule() -> std::unique_ptr<DMAInterface> = 0;
  virtual auto GetDMAModuleSetup() -> DMASetupInterface& = 0;
  virtual auto ExportLastModulesIfReadback()
      -> std::vector<std::unique_ptr<ReadBackModule>> = 0;
  virtual auto GetILAModule() -> std::unique_ptr<ILA> = 0;
  virtual auto IsMultiChannelStream(bool is_input, int stream_index,
                                    QueryOperationType operation_type)
      -> bool = 0;
  virtual auto GetMultiChannelParams(
      bool is_input, int stream_index, QueryOperationType operation_type,
      const std::vector<std::vector<int>>& operation_parameters)
      -> std::pair<int, int> = 0;
  virtual auto GetNodeCapacity(
      QueryOperationType operation_type,
      const std::vector<std::vector<int>>& operation_parameters)
      -> std::vector<int> = 0;
  virtual auto IsNodeConstrainedToFirstInPipeline(
      QueryOperationType operation_type) -> bool = 0;
  virtual auto IsOperationSorting(QueryOperationType operation_type)
      -> bool = 0;
  virtual auto GetMinSortingRequirements(QueryOperationType operation_type,
                                         const TableMetadata& table_data)
      -> std::vector<int> = 0;
};

}  // namespace orkhestrafs::dbmstodspi