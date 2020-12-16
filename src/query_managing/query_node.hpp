#pragma once
#include <string>
#include <vector>
#include <optional>
#include "operation_types.hpp"
struct QueryNode {
  std::vector<std::string> input_data_definition_files;
  std::vector<std::string> output_data_definition_files;
  operation_types::QueryOperation operation_type;
  std::optional<QueryNode*> next_node;
};