#pragma once
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "operation_types.hpp"

namespace query_scheduling_data {

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

struct QueryNode {
  std::vector<std::string> input_data_definition_files;
  std::vector<std::string> output_data_definition_files;
  operation_types::QueryOperation operation_type;
  std::vector<query_scheduling_data::QueryNode *> next_nodes;
  std::vector<query_scheduling_data::QueryNode *> previous_nodes;
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

typedef std::vector<operation_types::QueryOperation> ConfigurableModulesVector;

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