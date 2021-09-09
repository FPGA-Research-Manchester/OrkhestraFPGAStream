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

using orkhestrafs::core_interfaces::operation_types::QueryOperation;
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;

namespace orkhestrafs::core_interfaces::query_scheduling_data {

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

/**
 * @brief Struct for defining a query node.
 */
struct QueryNode {
  /// Input data files.
  std::vector<std::string> input_data_definition_files;
  /// Expected data files.
  std::vector<std::string> output_data_definition_files;
  /// Query operation.
  QueryOperationType operation_type;
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
      QueryOperationType operation,
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
        operation_parameters{std::move(parameters)},
        node_name{std::move(node_name)},
        is_checked{std::move(is_checked)} {};
};

/// Map of available functions.
const std::map<std::string, QueryOperationType> kSupportedFunctions = {
    {"kFilter", QueryOperationType::kFilter},
    {"kJoin", QueryOperationType::kJoin},
    {"kMergeSort", QueryOperationType::kMergeSort},
    {"kLinearSort", QueryOperationType::kLinearSort},
    {"kAddition", QueryOperationType::kAddition},
    {"kMultiplication", QueryOperationType::kMultiplication},
    {"kAggregationSum", QueryOperationType::kAggregationSum},
    {"kPassThrough", QueryOperationType::kPassThrough}};

/// All of the kept info between execution states for reading stream results.
struct StreamResultParameters {
  int stream_index;
  int output_id;
  std::string filename;
  bool check_results;
  std::vector<std::vector<int>> stream_specifications;

  StreamResultParameters(int stream_index, int output_id, std::string filename,
                         bool check_results,
                         std::vector<std::vector<int>> stream_specifications)
      : stream_index{stream_index},
        output_id{output_id},
        filename{std::move(filename)},
        check_results{check_results},
        stream_specifications{std::move(stream_specifications)} {};
};

/// Type definition of a collection of operation types for selecting bitstreams.
using ConfigurableModulesVector = std::vector<QueryOperation>;
/// Type definition for meta data required to read and write table records.
using RecordSizeAndCount = std::pair<int, int>;
/// For remembering which stream memory should be reused with another stream.
using MemoryReuseTargets = std::vector<std::pair<std::string, int>>;

}  // namespace orkhestrafs::core_interfaces::query_scheduling_data
