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
#include <vector>

#include "operation_types.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::core_interfaces::operation_types::QueryOperationType;

namespace orkhestrafs::sql_parsing {

struct TableColumn {
  ColumnDataType column_type;
  int column_size;
  std::string column_name;
};

/**
 * @brief Class to programmatically create queries.
 */
class SQLQueryCreator {
 private:
  std::unordered_map<std::string, bool> is_table_;
  int operation_counter_;
  std::vector<std::string> input_operations_;
  std::vector<std::string> output_operations_;

  auto RegisterOperation(QueryOperationType operation_type) -> std::string;

 public:
  auto ExportInputDef() -> std::string;
  auto RegisterTable(std::string filename, std::vector<TableColumn> columns,
                     int row_count) -> std::string;
  auto RegisterFilter(std::string input) -> std::string;
  auto RegisterSort(std::string input) -> std::string;
  auto RegisterJoin(std::string first_input, std::string first_join_key,
                    std::string second_input, std::string second_join_key)
      -> std::string;
  auto RegisterAddition(std::string input) -> std::string;
  auto RegisterMultiplication(std::string input) -> std::string;
  auto RegisterAggregation(std::string input) -> std::string;
  auto AddComparison(std::string filter_id) -> int;
  auto AddOr(std::string filter_id, std::vector<int> comparison_ids) -> int;
  auto AddAnd(std::string filter_id, std::vector<int> comparison_ids) -> int;
  auto AddNot(std::string filter_id, std::vector<int> comparison_ids) -> int;
};
}  // namespace orkhestrafs::core::core_execution