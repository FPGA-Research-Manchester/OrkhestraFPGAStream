#pragma once
#include <vector>

#include "accelerated_query_node.hpp"
#include "operation_types.hpp"
#include "stream_data_parameters.hpp"
struct AcceleratedQueryNode {
  const std::vector<StreamDataParameters> input_streams;
  const std::vector<StreamDataParameters> output_streams;
  const operation_types::QueryOperation operation_type;
  const std::vector <bool> is_input_intermediate;
  const std::vector <bool> is_output_intermediate;
};