/*
Copyright 2022 University of Manchester

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

#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "operation_types.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::table_data::ColumnDataType;

namespace orkhestrafs::sql_parsing::query_data {

enum class CompareFunctions {
  kLessThan = 0,
  kLessThanOrEqual = 1,
  kEqual = 2,
  kGreaterThanOrEqual = 3,
  kGreaterThan = 4,
  kNotEqual = 5,
};

struct FilterOperation {
  CompareFunctions operation;
  std::string column_name;
  std::variant<std::vector<int>, std::pair<std::string, int>> comparison_values;
  bool negated; // For future
};

struct TableColumn {
  ColumnDataType column_type;
  int column_size;
  std::string column_name;

  TableColumn(ColumnDataType column_type, int column_size,
              std::string column_name)
      : column_type(column_type),
        column_size(column_size),
        column_name(column_name){};
};

using OperationParams =
    std::vector<std::variant<std::vector<int>, std::pair<std::string, int>>>;

struct OperationData {
  QueryOperationType operation_type;
  std::string sorted_by_column;
  std::unordered_map<std::string, int> column_locations;
  std::vector<std::string> operation_columns;
  std::vector<std::string> desired_columns;
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
  OperationParams operation_params;
  std::vector<std::vector<int>> input_params;
  std::vector<std::vector<int>> output_params;
};

}  // namespace orkhestrafs::sql_parsing::query_data