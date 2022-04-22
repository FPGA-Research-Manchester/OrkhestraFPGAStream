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
  //  for (const auto& [process_name, is_table] : is_table_) {
  //    if (!is_table) {
  //      auto observe = operations_.at(process_name).desired_columns;
  //      auto copy = observe;
  //    }
  //  }
  UpdateRequiredColumns();
  //  for (const auto& [process_name, is_table] : is_table_) {
  //    if (!is_table) {
  //      auto observe = operations_.at(process_name).desired_columns;
  //      auto copy = observe;
  //    }
  //  }

  std::unordered_set<std::string> processed_operations;
  AddTablesToProcessedOperations(processed_operations);

  std::unordered_set<std::string> operations_to_process;
  for (const auto& process_name : input_operations_) {
    UpdateNextOperationsListIfAvailable(processed_operations,
                                        operations_to_process, process_name);
  }

  std::map<std::string, InputNodeParameters> data_to_write;
  FillDataMap(std::move(processed_operations), std::move(operations_to_process),
              data_to_write);
  // Add crossbar reduction to output nodes - Add all nodes before merge sort as
  // output nodes as well! data_to_write
  const std::string file_name = "Q19.json";
  SQLJSONWriter::WriteQuery(file_name, data_to_write);
  // return file_name;
  return "benchmark_Q19_SF001.json";
}
void SQLQueryCreator::FillDataMap(
    std::unordered_set<std::string> processed_operations,
    std::unordered_set<std::string> operations_to_process,
    std::map<std::string, InputNodeParameters>& data_to_write) {
  while (!operations_to_process.empty()) {
    auto current_process = *operations_to_process.begin();
    operations_to_process.erase(current_process);
    processed_operations.insert(current_process);

    InputNodeParameters current_parameters;
    SetInputsForDataMap(current_process, current_parameters);
    SetOutputsForDataMap(current_process, current_parameters,
                         processed_operations, operations_to_process);
    current_parameters.insert(
        {operation_string,
         operation_enum_strings_.at(
             operations_.at(current_process).operation_type)});
    SetStreamParamsForDataMap(current_process, current_parameters);

    data_to_write.insert({current_process, current_parameters});
  }
}
void SQLQueryCreator::SetStreamParamsForDataMap(
    const std::string& current_process,
    InputNodeParameters& current_parameters) {
  int record_size = SetIOStreamParams(current_process);
  // TODO: Remove magic numbers!
  operations_[current_process].output_params[chunk_count_offset].push_back(
      (record_size + 15) / 16);
  // Should just be for Join.
  CombineOutputStreamParams(current_process);

  std::map<std::string, OperationParams> current_operation_params;
  OperationParams params;
  for (const auto& param_vector : operations_[current_process].input_params) {
    params.push_back(param_vector);
  }
  current_operation_params.insert({input_parameters_string, params});
  params.clear();
  for (const auto& param_vector : operations_[current_process].output_params) {
    params.push_back(param_vector);
  }
  current_operation_params.insert({output_parameters_string, params});

  SetOperationSpecifcStreamParamsForDataMap(current_process,
                                            current_operation_params);
  current_parameters.insert(
      {operation_parameters_string, current_operation_params});
}

void SQLQueryCreator::SetOperationSpecifcStreamParamsForDataMap(
    std::string current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  // TODO: Currently set up to only do one column at a time.
  auto current_operation = operations_[current_process].operation_type;
  if (current_operation == QueryOperationType::kAddition) {
    if (operations_[current_process].column_locations.find(
            operations_[current_process].operation_columns[0]) ==
        operations_[current_process].column_locations.end()) {
      throw std::runtime_error("Addition column not found!");
    }
    auto column_location = operations_[current_process]
                               .column_locations[operations_[current_process]
                                                     .operation_columns[0]];
    if (column_location % 2) {
      throw std::runtime_error("Aggregation column incorrectly placed!");
    }
    OperationParams params;
    std::vector<int> chunk_vector = {column_location / 16};
    params.push_back(chunk_vector);

    auto given_operation_params = std::get<std::vector<int>>(
        operations_[current_process].operation_params[0]);
    std::vector<int> negation_vector(8, 0);
    if (given_operation_params[0]) {
      negation_vector[(column_location % 16) / 2] = 1;
    }
    params.push_back(negation_vector);

    std::vector<int> value_vector(16, 0);
    value_vector[(column_location % 16) + 1] = given_operation_params[1];
    params.push_back(value_vector);
    current_operation_params.insert({operation_specific_params_string, params});
  } else if (current_operation == QueryOperationType::kAggregationSum) {
    if (operations_[current_process].column_locations.find(
            operations_[current_process].operation_columns[0]) ==
        operations_[current_process].column_locations.end()) {
      throw std::runtime_error("Aggregation column not found!");
    }
    auto column_location = operations_[current_process]
                               .column_locations[operations_[current_process]
                                                     .operation_columns[0]];
    if (column_location % 2) {
      throw std::runtime_error("Aggregation column incorrectly placed!");
    }
    OperationParams params;
    std::vector<int> chunk_vector = {column_location / 16};
    params.push_back(chunk_vector);
    // TODO: Not sure if it has to be in range 16 or 8.
    std::vector<int> position_vector = {column_location % 16};
    params.push_back(position_vector);
    current_operation_params.insert({operation_specific_params_string, params});
  } else if (current_operation == QueryOperationType::kMultiplication) {
    if (operations_[current_process].column_locations.find(
            operations_[current_process].operation_columns[0]) ==
            operations_[current_process].column_locations.end() ||
        operations_[current_process].column_locations.find(
            operations_[current_process].operation_columns[1]) ==
            operations_[current_process].column_locations.end()) {
      throw std::runtime_error("Multiplication column not found!");
    }
    auto first_column_location = operations_[current_process].column_locations
                     [operations_[current_process].operation_columns[0]];
    auto second_column_location = operations_[current_process].column_locations
                     [operations_[current_process].operation_columns[1]];
    if (first_column_location / 16 != second_column_location / 16 ||
        (first_column_location % 4) || (second_column_location % 2) ||
        (first_column_location + 2 != second_column_location)) {
      throw std::runtime_error("Multiplication columns incorrectly placed!");
    }
    std::vector<int> params;
    // TODO: Hardcoded to overwrite the first column at the moment!
    operations_[current_process].column_locations.erase(
        operations_[current_process].operation_columns[0]);
    operations_[current_process].column_locations.insert(
        {operations_[current_process].operation_columns[2],
         first_column_location});
    params.push_back(first_column_location / 16);
    for (int i = 0; i < 8; i++) {
      if (i == (first_column_location % 16) / 2) {
        params.push_back(1);
      } else {
        params.push_back(0);
      }
    }
    current_operation_params.insert(
        {operation_specific_params_string, {params}});
  } else {
    current_operation_params.insert({operation_specific_params_string, {}});
  }
}

auto SQLQueryCreator::SetIOStreamParams(const std::string& current_process)
    -> int {
  // TODO: Still with multiple outputs the current approach doesn't work.
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
      //      auto observe0 = operations_[current_process].output_params[0];
      //      auto observe1 = operations_[current_process].output_params[1];
      //      auto observe2 = operations_[current_process].output_params[2];
      //      auto observe3 = operations_[current_process].output_params[3];
      int last_needed_column_index =
          ProcessTableColumns(current_process, parent_index, parent);
      //      observe0 = operations_[current_process].output_params[0];
      //      observe1 = operations_[current_process].output_params[1];
      //      observe2 = operations_[current_process].output_params[2];
      //      observe3 = operations_[current_process].output_params[3];
      record_size += CompressNullColumns(current_process, parent_index, parent,
                                         last_needed_column_index);
      //      observe0 = operations_[current_process].output_params[0];
      //      observe1 = operations_[current_process].output_params[1];
      //      observe2 = operations_[current_process].output_params[2];
      //      observe3 = operations_[current_process].output_params[3];
    } else {
      CopyOutputParamsOfParent(current_process, parent_index, parent);
      for (const auto& column : operations_[parent].desired_columns) {
        auto [data_type, data_size] = columns_.at(column);
        record_size += column_sizes_.at(data_type) * data_size;
      }
    }
    record_size -= GetDuplicatedColumnSizes(current_process, parent_index);
  }
  return record_size;
}
void SQLQueryCreator::CombineOutputStreamParams(
    const std::string& current_process) {
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
}
int SQLQueryCreator::GetDuplicatedColumnSizes(
    const std::string& current_process, int parent_index) {
  int duplicated_record_size = 0;
  if (operations_.at(current_process).operation_type ==
          QueryOperationType::kJoin &&
      parent_index == 1) {
    // Find first column of join.
    int data_type = operations_[current_process]
                        .output_params[parent_index * io_param_vector_count +
                                       data_types_offset]
                        .front();
    int data_size = operations_[current_process]
                        .output_params[parent_index * io_param_vector_count +
                                       data_sizes_offset]
                        .front();
    duplicated_record_size +=
        column_sizes_.at(static_cast<ColumnDataType>(data_type)) * data_size;
  }
  return duplicated_record_size;
}
void SQLQueryCreator::CopyOutputParamsOfParent(
    const std::string& current_process, int parent_index, std::string parent) {
  operations_[current_process]
      .input_params[parent_index * io_param_vector_count + data_types_offset] =
      operations_[parent].output_params[data_types_offset];
  operations_[current_process]
      .input_params[parent_index * io_param_vector_count + data_sizes_offset] =
      operations_[parent].output_params[data_sizes_offset];
  operations_[current_process]
      .output_params[parent_index * io_param_vector_count + data_types_offset] =
      operations_[parent].output_params[data_types_offset];
  operations_[current_process]
      .output_params[parent_index * io_param_vector_count + data_sizes_offset] =
      operations_[parent].output_params[data_sizes_offset];
  for (const auto& [column_name, column_location] :
       operations_[parent].column_locations) {
    operations_[current_process].column_locations.insert(
        {column_name, column_location});
  }
}
int SQLQueryCreator::CompressNullColumns(const std::string& current_process,
                                         int parent_index,
                                         std::string table_name,
                                         int last_needed_column_index) {
  int record_size = 0;
  std::vector<int> new_data_types_concat;
  std::vector<int> new_data_sizes_concat;
  int crossbar_config_length = 0;
  for (int column_index = 0; column_index <= last_needed_column_index;
       column_index++) {
    auto [data_type, data_size] =
        columns_.at(tables_.at(table_name).at(column_index));
    int column_length = column_sizes_.at(data_type) * data_size;
    record_size += column_length;
    int current_data_type =
        operations_[current_process]
            .output_params[parent_index * io_param_vector_count +
                           data_types_offset][column_index];
    if (!new_data_types_concat.empty() &&
        current_data_type == static_cast<int>(ColumnDataType::kNull) &&
        new_data_types_concat.back() ==
            static_cast<int>(ColumnDataType::kNull)) {
      new_data_sizes_concat.back() += column_length;
    } else {
      new_data_types_concat.push_back(current_data_type);
      new_data_sizes_concat.push_back(data_size);
    }
    crossbar_config_length += column_length;
  }
  operations_[current_process]
      .output_params[parent_index * io_param_vector_count + data_sizes_offset] =
      new_data_sizes_concat;
  operations_[current_process]
      .output_params[parent_index * io_param_vector_count + data_types_offset] =
      new_data_types_concat;
  operations_[current_process]
      .input_params[parent_index * io_param_vector_count + crossbar_offset]
      .resize(crossbar_config_length);
  return record_size;
}
int SQLQueryCreator::ProcessTableColumns(const std::string& current_process,
                                         int parent_index,
                                         std::string table_name) {
  int current_position_index = 0;
  int last_needed_column_index = 0;
  for (int column_index = 0; column_index < tables_.at(table_name).size();
       column_index++) {
    auto [data_type, data_size] =
        columns_.at(tables_.at(table_name).at(column_index));
    operations_[current_process]
        .input_params[parent_index * io_param_vector_count + data_types_offset]
        .push_back(static_cast<int>(data_type));
    operations_[current_process]
        .input_params[parent_index * io_param_vector_count + data_sizes_offset]
        .push_back(data_size);
    int column_length = column_sizes_.at(data_type) * data_size;
    if (std::find(operations_.at(current_process).desired_columns.begin(),
                  operations_.at(current_process).desired_columns.end(),
                  tables_.at(table_name).at(column_index)) !=
        operations_.at(current_process).desired_columns.end()) {
      operations_[current_process].column_locations.insert(
          {tables_.at(table_name).at(column_index), current_position_index});
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
  return last_needed_column_index;
}
void SQLQueryCreator::SetOutputsForDataMap(
    const std::string& current_process, InputNodeParameters& current_parameters,
    std::unordered_set<std::string>& processed_operations,
    std::unordered_set<std::string>& operations_to_process) {  
  // TODO: Breaks when a final node has multiple outputs!
  std::vector<std::string> output_files;
  std::vector<std::string> output_nodes;
  for (const auto& child : operations_.at(current_process).outputs) {
    // Currently, can't set output as a table
    output_files.push_back("");
    output_nodes.push_back(child);
    UpdateNextOperationsListIfAvailable(processed_operations,
                                        operations_to_process, child);
  }
  // Temporary workaround
  if (output_files.empty()) {
    output_files.push_back("");
    output_nodes.push_back("");
  }
  current_parameters.insert({output_files_string, output_files});
  current_parameters.insert({output_nodes_string, output_nodes});
}
void SQLQueryCreator::SetInputsForDataMap(
    const std::string& current_process,
    InputNodeParameters& current_parameters) {
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
}
void SQLQueryCreator::UpdateNextOperationsListIfAvailable(
    std::unordered_set<std::string>& processed_operations,
    std::unordered_set<std::string>& operations_to_process,
    const std::basic_string<char>& process_name) {
  if (std::all_of(operations_.at(process_name).inputs.begin(),
                  operations_.at(process_name).inputs.end(),
                  [&](const auto& input) {
                    return processed_operations.find(input) !=
                           processed_operations.end();
                  })) {
    operations_to_process.insert(process_name);
  }
}
void SQLQueryCreator::AddTablesToProcessedOperations(
    std::unordered_set<std::string>& processed_operations) {
  for (const auto& [table_name, is_table] : is_table_) {
    if (is_table) {
      processed_operations.insert(table_name);
    }
  }
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
        auto transferred_columns =
            operations_.at(current_process).desired_columns;
        // TODO: How used columns is handled in these two cases is broken!
        if (operations_.at(current_process).operation_type ==
            QueryOperationType::kJoin) {
          transferred_columns.erase(transferred_columns.begin());
        } else if (operations_.at(current_process).operation_type ==
                   QueryOperationType::kMultiplication) {
          /*transferred_columns.erase(transferred_columns.begin() + 2);
          operations_.at(current_process).desired_columns[0] =
              operations_.at(current_process).desired_columns[2];
          operations_.at(current_process)
              .desired_columns.erase(
                  operations_.at(current_process).desired_columns.begin() + 2);*/
        }
        for (const auto& column : transferred_columns) {
          if (std::find(operations_.at(parent).desired_columns.begin(),
                        operations_.at(parent).desired_columns.end(), column) ==
              operations_.at(parent).desired_columns.end()) {
            operations_.at(parent).desired_columns.push_back(column);
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
      column.column_size = ((column.column_size + 3) / 4) * 4;
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
    operations_[lin_sort_name].desired_columns.push_back(column_name);
    operations_[lin_sort_name].sorted_by_column = column_name;
    operations_[merge_sort_name].desired_columns.push_back(column_name);
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
  operations_[join_name].desired_columns.push_back(first_join_key);
  return join_name;
}

// Just one at a time for these at the moment.
auto SQLQueryCreator::RegisterAddition(std::string input,
                                       std::string column_name,
                                       bool make_negative, double value)
    -> std::string {
  auto new_name = RegisterOperation(QueryOperationType::kAddition, {input});
  operations_[new_name].desired_columns.push_back(column_name);
  operations_[new_name].operation_columns.push_back(column_name);
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
  columns_.insert({result_column_name, {ColumnDataType::kDecimal, 1}});
  auto new_name =
      RegisterOperation(QueryOperationType::kMultiplication, {input});
  operations_[new_name].desired_columns.push_back(first_column_name);
  operations_[new_name].desired_columns.push_back(second_column_name);
  operations_[new_name].operation_columns.push_back(first_column_name);
  operations_[new_name].operation_columns.push_back(second_column_name);
  operations_[new_name].operation_columns.push_back(result_column_name);
  return new_name;
}
auto SQLQueryCreator::RegisterAggregation(std::string input,
                                          std::string column_name)
    -> std::string {
  auto new_name =
      RegisterOperation(QueryOperationType::kAggregationSum, {input});
  operations_[new_name].desired_columns.push_back(column_name);
  operations_[new_name].operation_columns.push_back(column_name);
  return new_name;
}

auto SQLQueryCreator::AddStringComparison(std::string filter_id,
                                          std::string column_name,
                                          CompareFunctions comparison_type,
                                          std::string compare_value) -> int {
  if (std::find(operations_[filter_id].desired_columns.begin(),
                operations_[filter_id].desired_columns.end(),
                column_name) == operations_[filter_id].desired_columns.end()) {
    operations_[filter_id].desired_columns.push_back(column_name);
  }
  return 0;
}
auto SQLQueryCreator::AddDateComparison(std::string filter_id,
                                        std::string column_name,
                                        CompareFunctions comparison_type,
                                        int year, int month, int day) -> int {
  if (std::find(operations_[filter_id].desired_columns.begin(),
                operations_[filter_id].desired_columns.end(),
                column_name) == operations_[filter_id].desired_columns.end()) {
    operations_[filter_id].desired_columns.push_back(column_name);
  }
  return 0;
}
auto SQLQueryCreator::AddIntegerComparison(std::string filter_id,
                                           std::string column_name,
                                           CompareFunctions comparison_type,
                                           int compare_value) -> int {
  if (std::find(operations_[filter_id].desired_columns.begin(),
                operations_[filter_id].desired_columns.end(),
                column_name) == operations_[filter_id].desired_columns.end()) {
    operations_[filter_id].desired_columns.push_back(column_name);
  }
  return 0;
}
auto SQLQueryCreator::AddDoubleComparison(std::string filter_id,
                                          std::string column_name,
                                          CompareFunctions comparison_type,
                                          double compare_value) -> int {
  if (std::find(operations_[filter_id].desired_columns.begin(),
                operations_[filter_id].desired_columns.end(),
                column_name) == operations_[filter_id].desired_columns.end()) {
    operations_[filter_id].desired_columns.push_back(column_name);
  }
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