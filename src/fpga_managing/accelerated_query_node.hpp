#pragma once
#include <vector>

#include "accelerated_query_node.hpp"
#include "operation_types.hpp"
#include "stream_data_parameters.hpp"

/**
 * Struct to hold all of the important information about a query node to
 * accelerate it with an FPGA.
 */
struct AcceleratedQueryNode {
  const std::vector<StreamDataParameters> input_streams;
  const std::vector<StreamDataParameters> output_streams;
  const operation_types::QueryOperation operation_type;
  const std::vector<std::vector<int>> operation_parameters;
};