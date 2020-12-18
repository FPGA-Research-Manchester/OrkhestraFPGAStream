#pragma once
#include <vector>
#include "stream_data_parameters.hpp"
#include "accelerated_query_node.hpp"
#include "operation_types.hpp"
struct AcceleratedQueryNode {
  std::vector<StreamDataParameters> input_streams;
  std::vector<StreamDataParameters> output_streams;
  operation_types::QueryOperation operation_type;
  bool is_input_intermediate = false;
  bool is_output_intermediate = false;
};