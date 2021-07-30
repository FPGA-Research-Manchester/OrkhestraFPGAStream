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
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"

namespace dbmstodspi::query_managing::query_scheduling_data {

using fpga_managing::query_acceleration_constants::kModuleSize;

/**
 * @brief Struct for collecting all of the parameter vectors.
 */
struct NodeOperationParameters {
  std::vector<std::vector<int>> input_stream_parameters;
  std::vector<std::vector<int>> output_stream_parameters;
  std::vector<std::vector<int>> operation_parameters;

  auto operator==(const NodeOperationParameters& rhs) const -> bool {
    return input_stream_parameters == rhs.input_stream_parameters &&
           output_stream_parameters == rhs.output_stream_parameters &&
           operation_parameters == rhs.operation_parameters;
  }
};

const struct StreamParamDefinition {
  int kStreamParamCount = 4;
  int kProjectionOffset = 0;
  int kDataTypesOffset = 1;
  int kDataSizesOffset = 2;
  int kChunkCountOffset = 3;
} kIOStreamParamDefs;

/**
 * @brief Struct for defining a query node.
 */
struct QueryNode {
  /// Input data files.
  std::vector<std::string> input_data_definition_files;
  /// Expected data files.
  std::vector<std::string> output_data_definition_files;
  /// Query operation.
  fpga_managing::operation_types::QueryOperationType operation_type;
  /// Pointers to the next query nodes.
  std::vector<std::shared_ptr<query_scheduling_data::QueryNode>> next_nodes;
  /// Pointers to the prerequisite query nodes
  std::vector<std::weak_ptr<query_scheduling_data::QueryNode>> previous_nodes;
  /// Operation parameters to configure the streams with modules.
  NodeOperationParameters operation_parameters;
  /// Location of the module to be processing this node
  int module_location = -1;
  /// Name of the node for automatic file naming
  std::string node_name;
  /// Flag vector setting stream results to be checked
  std::vector<bool> is_checked;

  auto operator==(const QueryNode& rhs) const -> bool {
    bool are_prev_nodes_equal =
        previous_nodes.size() == rhs.previous_nodes.size() &&
        std::equal(
            previous_nodes.begin(), previous_nodes.end(),
            rhs.previous_nodes.begin(),
            [](const std::weak_ptr<query_scheduling_data::QueryNode>& l,
               const std::weak_ptr<query_scheduling_data::QueryNode>& r) {
              return l.lock() == r.lock();
            });

    return are_prev_nodes_equal &&
           input_data_definition_files == rhs.input_data_definition_files &&
           output_data_definition_files == rhs.output_data_definition_files &&
           operation_type == rhs.operation_type &&
           next_nodes == rhs.next_nodes &&
           operation_parameters == rhs.operation_parameters &&
           module_location == rhs.module_location &&
           node_name == rhs.node_name && is_checked == rhs.is_checked;
  }

  QueryNode(
      std::vector<std::string> input, std::vector<std::string> output,
      fpga_managing::operation_types::QueryOperationType operation,
      std::vector<std::shared_ptr<query_scheduling_data::QueryNode>> next_nodes,
      std::vector<std::weak_ptr<query_scheduling_data::QueryNode>>
          previous_nodes,
      NodeOperationParameters parameters, std::string node_name,
      std::vector<bool> is_checked)
      : input_data_definition_files{std::move(input)},
        output_data_definition_files{std::move(output)},
        operation_type{operation},
        next_nodes{std::move(next_nodes)},
        previous_nodes{std::move(previous_nodes)},
        operation_parameters{parameters},
        node_name{node_name},
        is_checked{is_checked} {};
};

/// Type definition of a collection of operation types for selecting bitstreams.
using ConfigurableModulesVector =
    std::vector<fpga_managing::operation_types::QueryOperation>;

/// Map of available functions.
const std::map<std::string, fpga_managing::operation_types::QueryOperationType>
    kSupportedFunctions = {
        {"kFilter",
         fpga_managing::operation_types::QueryOperationType::kFilter},
        {"kJoin", fpga_managing::operation_types::QueryOperationType::kJoin},
        {"kMergeSort",
         fpga_managing::operation_types::QueryOperationType::kMergeSort},
        {"kLinearSort",
         fpga_managing::operation_types::QueryOperationType::kLinearSort},
        {"kAddition",
         fpga_managing::operation_types::QueryOperationType::kAddition},
        {"kMultiplication",
         fpga_managing::operation_types::QueryOperationType::kMultiplication},
        {"kAggregationSum",
         fpga_managing::operation_types::QueryOperationType::kAggregationSum},
        {"kPassThrough",
         fpga_managing::operation_types::QueryOperationType::kPassThrough}};
}  // namespace dbmstodspi::query_managing::query_scheduling_data
