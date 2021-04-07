#pragma once
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "operation_types.hpp"
/**
 * @brief Namespace for types to help with scheduling the query nodes correctly.
 */
namespace query_scheduling_data {

/**
 * @brief Struct for collecting all of the parameter vectors.
 */
struct NodeOperationParameters {
  std::vector<std::vector<int>> input_stream_parameters;
  std::vector<std::vector<int>> output_stream_parameters;
  std::vector<std::vector<int>> operation_parameters;

  bool operator==(const NodeOperationParameters &rhs) const {
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
  operation_types::QueryOperation operation_type;
  /// Pointers to the next query nodes.
  std::vector<query_scheduling_data::QueryNode *> next_nodes;
  /// Pointers to the prerequisite query nodes
  std::vector<query_scheduling_data::QueryNode *> previous_nodes;
  /// Operation parameters to configure the streams with modules.
  NodeOperationParameters operation_parameters;

  bool operator==(const QueryNode &rhs) const {
    return input_data_definition_files == rhs.input_data_definition_files &&
           output_data_definition_files == rhs.output_data_definition_files &&
           operation_type == rhs.operation_type &&
           next_nodes == rhs.next_nodes &&
           previous_nodes == rhs.previous_nodes &&
           operation_parameters == rhs.operation_parameters;
  }
};

/// Type definition of a collection of operation types for selecting bitstreams.
typedef std::vector<operation_types::QueryOperation> ConfigurableModulesVector;

/// Map of supported collections of operations.
const std::map<ConfigurableModulesVector, std::string>
    supported_accelerator_bitstreams = {
        {{operation_types::QueryOperation::kFilter}, "DSPI_filtering"},

        {{operation_types::QueryOperation::kJoin}, "DSPI_joining"},

        /*{{operation_types::QueryOperation::kMergeSort,
          operation_types::QueryOperation::kMergeSort},
         "DSPI_double_merge_sorting"},*/

        {{operation_types::QueryOperation::kMergeSort}, "DSPI_merge_sorting"},

        {{operation_types::QueryOperation::kLinearSort}, "DSPI_linear_sorting"},

        {{}, "DSPI_empty"}/*,

        {{operation_types::QueryOperation::kFilter,
          operation_types::QueryOperation::kJoin},
         "DSPI_filter_join"},*/

        /*{{operation_types::QueryOperation::kFilter,
          operation_types::QueryOperation::kLinearSort},
         "DSPI_filtering_linear_sort"}*/};
}  // namespace query_scheduling_data