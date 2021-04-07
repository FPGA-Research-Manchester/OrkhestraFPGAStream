#pragma once
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "operation_types.hpp"

namespace dbmstodspi {
namespace query_managing {

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
  fpga_managing::operation_types::QueryOperation operation_type;
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
typedef std::vector<fpga_managing::operation_types::QueryOperation>
    ConfigurableModulesVector;

/// Map of supported collections of operations.
const std::map<ConfigurableModulesVector, std::string>
    supported_accelerator_bitstreams = {
        {{fpga_managing::operation_types::QueryOperation::kFilter}, "DSPI_filtering"},

        {{fpga_managing::operation_types::QueryOperation::kJoin}, "DSPI_joining"},

        /*{{fpga_managing::operation_types::QueryOperation::kMergeSort,
          fpga_managing::operation_types::QueryOperation::kMergeSort},
         "DSPI_double_merge_sorting"},*/

        {{fpga_managing::operation_types::QueryOperation::kMergeSort}, "DSPI_merge_sorting"},

        {{fpga_managing::operation_types::QueryOperation::kLinearSort}, "DSPI_linear_sorting"},

        {{}, "DSPI_empty"}/*,

        {{fpga_managing::operation_types::QueryOperation::kFilter,
          fpga_managing::operation_types::QueryOperation::kJoin},
         "DSPI_filter_join"},*/

        /*{{fpga_managing::operation_types::QueryOperation::kFilter,
          fpga_managing::operation_types::QueryOperation::kLinearSort},
         "DSPI_filtering_linear_sort"}*/};
}  // namespace query_scheduling_data

}  // namespace query_managing
}  // namespace dbmstodspi