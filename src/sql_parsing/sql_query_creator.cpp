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

#include "sql_json_writer.hpp"

using orkhestrafs::sql_parsing::SQLJSONWriter;
using orkhestrafs::sql_parsing::SQLQueryCreator;

auto SQLQueryCreator::ExportInputDef() -> std::string {
  UpdateRequiredColumns();
  std::unordered_set<std::string> processed_operations;
  auto operations_to_process = input_operations_;
  std::map<std::string, InputNodeParameters> data_to_write;

  while (!operations_to_process.empty()) {
    auto current_process = *operations_to_process.begin();
    operations_to_process.erase(current_process);
    processed_operations.insert(current_process);
    InputNodeParameters current_parameters;

    std::vector<std::string> input_files;
    std::vector<std::string> input_nodes;
    for (const auto& parent : operations_.at(current_process).outputs) {
      if (is_table_.at(parent)) {
        input_files.push_back(parent);
        input_nodes.push_back("");
      } else {
        input_nodes.push_back(parent);
        input_files.push_back("");
      }
    }
    current_parameters.insert({input_files_string, input_files});
    current_parameters.insert({input_nodes_string, input_nodes});
    current_parameters.insert(
        {operation_string,
         operation_enum_strings_.at(
             operations_.at(current_process).operation_type)});
    // TODO: Breaks when a final node has multiple outputs!
    std::vector<std::string> output_files;
    std::vector<std::string> output_nodes;
    for (const auto& child : operations_.at(current_process).outputs) {
      // Currently, can't set output as a table
      output_files.push_back("");
      output_nodes.push_back(child);
      if (std::all_of(operations_.at(child).inputs.begin(),
                      operations_.at(child).inputs.end(),
                      [&](const auto& input) {
                        return processed_operations.find(input) !=
                               processed_operations.end();
                      })) {
        operations_to_process.insert(child);
      }
    }
    // Temporary workaround
    if (output_files.empty()) {
      output_files.push_back("");
      output_nodes.push_back("");
    }
    current_parameters.insert({output_files_string, output_files});
    current_parameters.insert({output_nodes_string, output_nodes});

    // TODO Still with multiple outputs the current approach doesn't work.
    int record_size = 0;
    for (int parent_index = 0;
         parent_index < operations_.at(current_process).inputs.size();
         parent_index++) {
      auto parent = operations_.at(current_process).inputs.at(parent_index);
      operations_[current_process].input_params.insert(
          operations_[current_process].input_params.end(), {{}, {}, {}, {}});
      operations_[current_process].output_params.insert(
          operations_[current_process].output_params.end(), {{}, {}, {}, {}});
      if (is_table_.at(parent)) {
        int last_needed_column_index = 0;
        int current_position_index = 0;
        for (int column_index = 0; column_index < tables_.at(parent).size();
             column_index++) {
          auto [data_type, data_size] =
              columns_.at(tables_.at(parent).at(column_index));
          operations_[current_process]
              .input_params[parent_index * io_param_vector_count +
                            data_types_offset]
              .push_back(static_cast<int>(data_type));
          operations_[current_process]
              .input_params[parent_index * io_param_vector_count +
                            data_sizes_offset]
              .push_back(data_size);
          int column_length = column_sizes_.at(data_type) * data_size;
          if (std::find(operations_.at(current_process).used_columns.begin(),
                        operations_.at(current_process).used_columns.end(),
                        tables_.at(parent).at(column_index)) !=
              operations_.at(current_process).used_columns.end()) {
            last_needed_column_index = column_index;
            for (int i = 0; i < column_length; i++) {
              operations_[current_process]
                  .input_params[parent_index * io_param_vector_count +
                                crossbar_offset]
                  .push_back(current_position_index++);
            }
            // Hardcoded 0 index to only support a single output.
            operations_[current_process]
                .output_params[parent_index * io_param_vector_count +
                               data_types_offset]
                .push_back(static_cast<int>(data_type));
            operations_[current_process]
                .output_params[parent_index * io_param_vector_count +
                               data_sizes_offset]
                .push_back(data_size);
          } else {
            for (int i = 0; i < column_length; i++) {
              operations_[current_process]
                  .input_params[parent_index * io_param_vector_count +
                                crossbar_offset]
                  .push_back(-1);
              current_position_index++;
            }
            operations_[current_process]
                .output_params[parent_index * io_param_vector_count +
                               data_types_offset]
                .push_back(static_cast<int>(ColumnDataType::kNull));
            operations_[current_process]
                .output_params[parent_index * io_param_vector_count +
                               data_sizes_offset]
                .push_back(column_length);
          }
        }

        std::vector<int> new_data_types_concat;
        std::vector<int> new_data_sizes_concat;
        int crossbar_config_length = 0;
        for (int column_index = 0; column_index < last_needed_column_index;
             column_index++) {
          auto [data_type, data_size] =
              columns_.at(tables_.at(parent).at(column_index));
          int column_length = column_sizes_.at(data_type) * data_size;
          record_size += column_length;
          if (data_type == ColumnDataType::kNull &&
              new_data_types_concat.back() ==
                  static_cast<int>(ColumnDataType::kNull)) {
            new_data_sizes_concat.back() += column_length;
          } else {
            new_data_types_concat.push_back(static_cast<int>(data_type));
            new_data_sizes_concat.push_back(data_size);
          }
          crossbar_config_length += column_length;
        }
        operations_[current_process]
            .output_params[parent_index * io_param_vector_count +
                           data_sizes_offset] = new_data_sizes_concat;
        operations_[current_process]
            .output_params[parent_index * io_param_vector_count +
                           data_types_offset] = new_data_types_concat;
        operations_[current_process]
            .input_params[parent_index * io_param_vector_count +
                          crossbar_offset]
            .resize(crossbar_config_length);
      } else {
        operations_[current_process]
            .input_params[parent_index * io_param_vector_count +
                          data_types_offset] =
            operations_[parent].output_params[data_types_offset];
        operations_[current_process]
            .input_params[parent_index * io_param_vector_count +
                          data_sizes_offset] =
            operations_[parent].output_params[data_sizes_offset];
        operations_[current_process]
            .output_params[parent_index * io_param_vector_count +
                           data_types_offset] =
            operations_[parent].output_params[data_types_offset];
        operations_[current_process]
            .output_params[parent_index * io_param_vector_count +
                           data_sizes_offset] =
            operations_[parent].output_params[data_sizes_offset];
        for (const auto& column : operations_[parent].used_columns) {
          auto [data_type, data_size] = columns_.at(column);
          record_size += column_sizes_.at(data_type) * data_size;
        }
      }
    }
    // TODO: Remove magic numbers!
    operations_[current_process].output_params[chunk_count_offset].push_back(
        (record_size + 15) / 16);
    for (int parent_index = 1;
         parent_index < operations_.at(current_process).inputs.size();
         parent_index++) {
      int start_index = 0;
      if (operations_.at(current_process).operation_type ==
          QueryOperationType::kJoin) {
        start_index++;
      }
      for (int i = start_index;
           i < operations_[current_process]
                   .output_params[parent_index * io_param_vector_count +
                                  data_sizes_offset]
                   .size();
           i++) {
        operations_[current_process].output_params[data_sizes_offset].push_back(
            operations_[current_process]
                .output_params[parent_index * io_param_vector_count +
                               data_sizes_offset][i]);
        operations_[current_process].output_params[data_types_offset].push_back(
            operations_[current_process]
                .output_params[parent_index * io_param_vector_count +
                               data_types_offset][i]);
      }
    }
    operations_[current_process].output_params.resize(io_param_vector_count);

    std::map<std::string, OperationParams> current_operation_params;
    OperationParams params;
    for (const auto& param_vector : operations_[current_process].input_params) {
      params.push_back(param_vector);
    }
    current_operation_params.insert({input_parameters_string, params});
    params.clear();
    for (const auto& param_vector :
         operations_[current_process].output_params) {
      params.push_back(param_vector);
    }
    current_operation_params.insert({output_parameters_string, params});
    // TODO: Fill in later!
    current_operation_params.insert({operation_specific_params_string, {}});
    current_parameters.insert(
        {operation_parameters_string, current_operation_params});
    data_to_write.insert({current_process, current_parameters});
  }
  // Add crossbar reduction to output nodes - Add all nodes before merge sort as
  // output nodes as well! data_to_write
  const std::string file_name = "Q19.json";
  SQLJSONWriter::WriteQuery(file_name, data_to_write);
  return file_name;
}

void SQLQueryCreator::UpdateRequiredColumns() {
  auto operations_to_process = output_operations_;
  std::unordered_set<std::string> processed_operations;
  while (!operations_to_process.empty()) {
    auto current_process = *operations_to_process.begin();
    operations_to_process.erase(current_process);
    processed_operations.insert(current_process);
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
        if (std::all_of(operations_.at(parent).outputs.begin(),
                        operations_.at(parent).outputs.end(),
                        [&](const auto& output) {
                          return processed_operations.find(output) !=
                                 processed_operations.end();
                        })) {
          operations_to_process.insert(parent);
        }
      }
    }
  }
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
  auto second_join_table = RegisterSort(second_input, second_join_key);
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