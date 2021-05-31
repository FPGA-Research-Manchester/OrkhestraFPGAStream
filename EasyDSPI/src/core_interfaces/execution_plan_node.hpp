#pragma once

#include <memory>
#include <string>
#include <vector>

/// Don't know if this even has to be defined

namespace easydspi::core_interfaces {
struct NodeOperationParameters {
  std::vector<std::vector<int>> input_stream_parameters;
  std::vector<std::vector<int>> output_stream_parameters;
  std::vector<std::vector<int>> operation_parameters;

  auto operator==(const NodeOperationParameters &rhs) const -> bool {
    return input_stream_parameters == rhs.input_stream_parameters &&
           output_stream_parameters == rhs.output_stream_parameters &&
           operation_parameters == rhs.operation_parameters;
  }
};

struct ExecutionPlanNode {
  // Need to be vectors
  std::string input_data_filename;
  std::string output_data_filename;
  std::shared_ptr<ExecutionPlanNode> next_node;
  std::shared_ptr<ExecutionPlanNode> previous_node;

  std::string operation_type;
  NodeOperationParameters operation_parameters;
  int module_location = -1;

  auto operator==(const ExecutionPlanNode &rhs) const -> bool {
    return input_data_filename == rhs.input_data_filename &&
           output_data_filename == rhs.output_data_filename &&
           operation_type == rhs.operation_type && next_node == rhs.next_node &&
           previous_node == rhs.previous_node &&
           operation_parameters == rhs.operation_parameters &&
           module_location == rhs.module_location;
  }

  ExecutionPlanNode(std::string input_data_filename,
                    std::string output_data_filename,
                    std::shared_ptr<ExecutionPlanNode> next_node,
                    std::shared_ptr<ExecutionPlanNode> previous_node,
                    std::string operation_type,
                    const NodeOperationParameters& operation_parameters)
      : input_data_filename{input_data_filename},
        output_data_filename{output_data_filename},
        next_node{next_node},
        previous_node{previous_node},
        operation_type{operation_type},
        operation_parameters{operation_parameters} {}
};
}  // namespace easydspi::core_interfaces

struct QueryNode {};