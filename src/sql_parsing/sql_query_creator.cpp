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

#include "sql_query_creator.hpp"

#include <algorithm>
#include <iostream>

using orkhestrafs::sql_parsing::SQLQueryCreator;
using orkhestrafs::sql_parsing::query_data::OperationParams;

using InputNodeParameters =
    std::map<std::string, std::variant<std::string, std::vector<std::string>,
                                       std::map<std::string, OperationParams>>>;

auto SQLQueryCreator::ExportInputDef() -> std::string {
  auto operations_to_process = output_operations_;
  while (!operations_to_process.empty()) {
    auto current_process = *operations_to_process.begin();
    operations_to_process.erase(current_process);
    // IF operation is join. Then the column of the second one is not needed in
    // the second input (not in first either) Basically if join don't put the
    // extra first element.
    //
    // If operation is Multiplication. Put the first two into previous. For the
    // current one - replace first with third one. and remove third.
    for (const auto& parent : operations_.at(current_process).inputs) {
      if (!is_table_.at(parent)) {
        auto transferred_columns = operations_.at(current_process).used_columns;
        if (operations_.at(current_process).operation_type ==
            QueryOperationType::kJoin) {
          transferred_columns.erase(transferred_columns.begin());
        } else if (operations_.at(current_process).operation_type ==
                   QueryOperationType::kMultiplication) {
          transferred_columns.erase(transferred_columns.begin() + 2);
          operations_.at(current_process).used_columns[0] =
              operations_.at(current_process).used_columns[2];
          operations_.at(current_process)
              .used_columns.erase(transferred_columns.begin() + 2);
        }
        for (const auto& column : transferred_columns) {
          if (std::find(operations_.at(parent).used_columns.begin(),
                        operations_.at(parent).used_columns.end(),
                        column) == operations_.at(parent).used_columns.end()) {
            operations_.at(parent).used_columns.push_back(column);
          }
        }
        // TODO: Have to check if the other childs have been done as well - need
        // to keep track of done stuff!
        operations_to_process.insert(parent);
      }
    }
  }
  std::map<std::string, InputNodeParameters> data_to_write;
  operations_to_process = input_operations_;
  // TODO: Now iterate all nodes from input to end building the stuff to write
  // into map. For now you can try to
  while (!operations_to_process.empty()) {
    auto current_process = *operations_to_process.begin();
    operations_to_process.erase(current_process);
  }
  // TODO Need a mechanism to write JSON - Make a method that accepts
  // data_to_write
  return "benchmark_Q19_SF001.json";
}

auto SQLQueryCreator::RegisterOperation(QueryOperationType operation_type,
                                        std::vector<std::string> inputs)
    -> std::string {
  std::string new_name = operation_names_.at(operation_type) + "_" +
                         std::to_string(operation_counter_++);
  is_table_.insert({new_name, false});
  if (std::any_of(inputs.begin(), inputs.end(), [&](const auto& input_name) {
        return is_table_.at(input_name);
      })) {
    input_operations_.insert(new_name);
  }
  output_operations_.insert(new_name);

  OperationData current_data;
  current_data.operation_type = operation_type;

  current_data.inputs = inputs;
  for (const auto& input_name : inputs) {
    output_operations_.erase(input_name);
    if (!is_table_.at(input_name)) {
      operations_[input_name].outputs.push_back(new_name);
    }
  }

  if (is_table_.at(inputs.front())) {
    current_data.sorted_by_column = tables_.at(inputs.front()).front();
  } else {
    current_data.sorted_by_column =
        operations_.at(inputs.front()).sorted_by_column;
  }

  operations_.insert({new_name, current_data});
  return new_name;
}

auto SQLQueryCreator::RegisterTable(std::string filename,
                                    std::vector<TableColumn> columns,
                                    int row_count) -> std::string {
  std::vector<std::string> column_names;
  for (auto& column : columns) {
    column_names.push_back(column.column_name);
    if (column.column_type == ColumnDataType::kVarchar) {
      column.column_size = (column.column_size + 3) / 4;
    }
    columns_.insert(
        {column.column_name, {column.column_type, column.column_size}});
  }
  tables_.insert({filename, column_names});
  is_table_.insert({filename, true});
  return filename;
}

auto SQLQueryCreator::RegisterFilter(std::string input) -> std::string {
  auto new_name = RegisterOperation(QueryOperationType::kFilter, {input});
  return new_name;
}

auto SQLQueryCreator::RegisterSort(std::string input, std::string column_name)
    -> std::string {
  if ((is_table_.at(input) && tables_.at(input).front() == column_name) ||
      (!is_table_.at(input) &&
       operations_.at(input).sorted_by_column == column_name)) {
    return input;
  } else {
    auto lin_sort_name =
        RegisterOperation(QueryOperationType::kLinearSort, {input});
    auto merge_sort_name =
        RegisterOperation(QueryOperationType::kMergeSort, {lin_sort_name});
    operations_[lin_sort_name].used_columns.push_back(column_name);
    operations_[lin_sort_name].sorted_by_column = column_name;
    operations_[merge_sort_name].used_columns.push_back(column_name);
    operations_[merge_sort_name].sorted_by_column = column_name;
    return merge_sort_name;
  }
}

auto SQLQueryCreator::RegisterJoin(std::string first_input,
                                   std::string first_join_key,
                                   std::string second_input,
                                   std::string second_join_key) -> std::string {
  auto first_join_table = RegisterSort(first_input, first_join_key);
  //  if ((is_table_.at(first_input) && tables_.at(first_input).front() ==
  //  first_join_key) ||
  //      (!is_table_.at(first_input) &&
  //       operations_.at(first_input).sorted_by_column == first_join_key)) {
  //    first_join_table = first_input;
  //  } else {
  //    auto lin_sort_name =
  //        RegisterOperation(QueryOperationType::kLinearSort, {first_input});
  //    first_join_table =
  //        RegisterOperation(QueryOperationType::kMergeSort, {lin_sort_name});
  //    operations_[lin_sort_name].used_columns.push_back(first_join_key);
  //    operations_[lin_sort_name].sorted_by_column = first_join_key;
  //    operations_[first_join_table].used_columns.push_back(first_join_key);
  //    operations_[first_join_table].sorted_by_column = first_join_key;
  //  }

  auto second_join_table = RegisterSort(second_input, second_join_key);
  //  if ((is_table_.at(second_input) && tables_.at(second_input).front() ==
  //  second_join_key) ||
  //      (!is_table_.at(second_input) &&
  //       operations_.at(second_input).sorted_by_column == second_join_key)) {
  //    second_join_table = second_input;
  //  } else {
  //    auto second_lin_sort_name =
  //        RegisterOperation(QueryOperationType::kLinearSort, {second_input});
  //    auto second_merge_sort_name = RegisterOperation(
  //        QueryOperationType::kMergeSort, {second_lin_sort_name});
  //    operations_[second_lin_sort_name].used_columns.push_back(second_join_key);
  //    operations_[second_lin_sort_name].sorted_by_column = second_join_key;
  //    operations_[second_merge_sort_name].used_columns.push_back(second_join_key);
  //    operations_[second_merge_sort_name].sorted_by_column = second_join_key;
  //  }
  auto join_name = RegisterOperation(QueryOperationType::kJoin,
                                     {first_join_table, second_join_table});
  operations_[join_name].used_columns.push_back(first_join_key);
  return join_name;
}

// Just one at a time for these at the moment.
auto SQLQueryCreator::RegisterAddition(std::string input,
                                       std::string column_name,
                                       bool make_negative, double value)
    -> std::string {
  auto new_name = RegisterOperation(QueryOperationType::kAddition, {input});
  operations_[new_name].used_columns.push_back(column_name);
  std::vector<int> initial_addition_values = {make_negative,
                                              static_cast<int>(value * 100)};
  operations_[new_name].operation_params.push_back(initial_addition_values);
  return new_name;
}
auto SQLQueryCreator::RegisterMultiplication(std::string input,
                                             std::string first_column_name,
                                             std::string second_column_name,
                                             std::string result_column_name)
    -> std::string {
  auto new_name =
      RegisterOperation(QueryOperationType::kMultiplication, {input});
  operations_[new_name].used_columns.push_back(first_column_name);
  operations_[new_name].used_columns.push_back(second_column_name);
  operations_[new_name].used_columns.push_back(result_column_name);
  return new_name;
}
auto SQLQueryCreator::RegisterAggregation(std::string input,
                                          std::string column_name)
    -> std::string {
  auto new_name =
      RegisterOperation(QueryOperationType::kAggregationSum, {input});
  operations_[new_name].used_columns.push_back(column_name);
  return new_name;
}

auto SQLQueryCreator::AddStringComparison(std::string filter_id,
                                          std::string column_name,
                                          CompareFunctions comparison_type,
                                          std::string compare_value) -> int {
  return 0;
}
auto SQLQueryCreator::AddDateComparison(std::string filter_id,
                                        std::string column_name,
                                        CompareFunctions comparison_type,
                                        int year, int month, int day) -> int {
  return 0;
}
auto SQLQueryCreator::AddIntegerComparison(std::string filter_id,
                                           std::string column_name,
                                           CompareFunctions comparison_type,
                                           int compare_value) -> int {
  return 0;
}
auto SQLQueryCreator::AddDoubleComparison(std::string filter_id,
                                          std::string column_name,
                                          CompareFunctions comparison_type,
                                          double compare_value) -> int {
  return 0;
}

auto SQLQueryCreator::AddOr(std::string filter_id,
                            std::vector<int> comparison_ids) -> int {
  return 0;
}

auto SQLQueryCreator::AddAnd(std::string filter_id,
                             std::vector<int> comparison_ids) -> int {
  return 0;
}

auto SQLQueryCreator::AddNot(std::string filter_id,
                             std::vector<int> comparison_ids) -> int {
  return 0;
}