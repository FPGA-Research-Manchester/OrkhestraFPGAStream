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

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::table_data::ColumnDataType;

namespace orkhestrafs::sql_parsing {

enum class CompareFunctions {
  kLessThan = 0,
  kLessThanOrEqual = 1,
  kEqual = 2,
  kGreaterThanOrEqual = 3,
  kGreaterThan = 4,
  kNotEqual = 5,
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
  auto RegisterSort(std::string input, std::string column_name) -> std::string;
  auto RegisterJoin(std::string first_input, std::string first_join_key,
                    std::string second_input, std::string second_join_key)
      -> std::string;
  auto RegisterAddition(std::string input, std::string column_name,
                        bool make_negative, double value) -> std::string;
  auto RegisterMultiplication(std::string input, std::string first_column_name,
                              std::string second_column_name,
                              std::string result_column_name) -> std::string;
  auto RegisterAggregation(std::string input, std::string column_name)
      -> std::string;
  auto AddStringComparison(std::string filter_id, std::string column_name,
                           CompareFunctions comparison_type,
                           std::string compare_value) -> int;
  auto AddDateComparison(std::string filter_id, std::string column_name,
                         CompareFunctions comparison_type, int year, int month,
                         int day) -> int;
  auto AddIntegerComparison(std::string filter_id, std::string column_name,
                            CompareFunctions comparison_type, int compare_value)
      -> int;
  auto AddDoubleComparison(std::string filter_id, std::string column_name,
                           CompareFunctions comparison_type,
                           double compare_value) -> int;
  auto AddOr(std::string filter_id, std::vector<int> comparison_ids) -> int;
  auto AddAnd(std::string filter_id, std::vector<int> comparison_ids) -> int;
  auto AddNot(std::string filter_id, std::vector<int> comparison_ids) -> int;
};
}  // namespace orkhestrafs::sql_parsing