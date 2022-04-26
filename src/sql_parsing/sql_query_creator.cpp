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
#include <set>
#include <stdexcept>
#include <utility>

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

  SetOperationSpecificStreamParamsForDataMap(current_process,
                                             current_operation_params);
  current_parameters.insert(
      {operation_parameters_string, current_operation_params});
}

void SQLQueryCreator::SetOperationSpecificStreamParamsForDataMap(
    std::string current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  // TODO: Currently set up to only do one column at a time.
  auto current_operation = operations_[current_process].operation_type;
  if (current_operation == QueryOperationType::kAddition) {
    SetAdditionStreamParams(current_process, current_operation_params);
  } else if (current_operation == QueryOperationType::kAggregationSum) {
    SetAggregationStreamParams(current_process, current_operation_params);
  } else if (current_operation == QueryOperationType::kMultiplication) {
    SetMultiplicationStreamParams(current_process, current_operation_params);
  } else if (current_operation == QueryOperationType::kFilter) {
    SetFilterStreamParams(current_process, current_operation_params);
  } else {
    current_operation_params.insert({operation_specific_params_string, {}});
  }
}

void SQLQueryCreator::SetFilterStreamParams(
    const std::string& current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  int new_base_clause_id = TransofrmToDNF(current_process, 0);
  std::set<std::vector<int>> all_clauses;
  for (const auto& child_clause_id :
       filter_operations_relations_[current_process][new_base_clause_id]) {
    if (IsLiteral(current_process, child_clause_id)) {
      all_clauses.insert({child_clause_id});
    } else {
      auto new_clause =
          filter_operations_relations_[current_process][child_clause_id];
      std::sort(new_clause.begin(), new_clause.end());
      all_clauses.insert(new_clause);
    }
  }

  std::unordered_map<std::string, std::vector<int>> operations_on_a_column;
  std::unordered_map<int, std::vector<int>> clauses;

  for (const auto& [literal_id, literal_params] :
       filter_operations_[current_process]) {
    if (operations_on_a_column.find(literal_params.column_name) ==
        operations_on_a_column.end()) {
      operations_on_a_column.insert({literal_params.column_name, {literal_id}});
    } else {
      if (std::find(operations_on_a_column[literal_params.column_name].begin(),
                    operations_on_a_column[literal_params.column_name].end(),
                    literal_id) ==
          operations_on_a_column[literal_params.column_name].end()) {
        operations_on_a_column[literal_params.column_name].push_back(
            literal_id);
      }
    }

    std::vector<int> current_clauses;
    for (int clause_id = 0; clause_id < all_clauses.size(); clause_id++) {
      auto observed_clause = *std::next(all_clauses.begin(), clause_id);
      if (std::find(observed_clause.begin(), observed_clause.end(),
                    literal_id) != observed_clause.end()) {
        current_clauses.push_back(clause_id);
      }
    }
    if (current_clauses.empty()) {
      throw std::runtime_error("Some literals were unused!");
    } else {
      clauses.insert({literal_id, current_clauses});
    }
  }

  OperationParams params;
  for (const auto& [column_name, literal_ids] : operations_on_a_column) {
    std::vector<int> location_vector;
    auto location = operations_[current_process].column_locations[column_name];
    location_vector.push_back(location / 16);
    location_vector.push_back(15 - (location % 16));
    location_vector.push_back(literal_ids.size());
    params.push_back(location_vector);
    for (const auto& literal_id : literal_ids) {
      std::vector<int> function_vector;
      function_vector.push_back(compare_function_mapping.at(
          filter_operations_[current_process][literal_id].operation));
      params.push_back(function_vector);
      if (std::holds_alternative<std::pair<std::string, int>>(
              filter_operations_[current_process][literal_id]
                  .comparison_values)) {
        params.push_back(std::get<std::pair<std::string, int>>(
            filter_operations_[current_process][literal_id].comparison_values));
      } else {
        auto current_values = std::get<std::vector<int>>(
            filter_operations_[current_process][literal_id].comparison_values);
        if (current_values.size() == 1) {
          std::vector<int> value_vector = {current_values.front()};
          params.push_back(value_vector);
        } else {
          // Need to make an identical hardcoded comparisson for decimal values.
          // In the future make it better!
          // TODO: Finish this!
        }
      }

      std::vector<int> clause_type_vector;
      // TODO: We don't do Negation at the moment.
      params.push_back(clause_type_vector);
      params.push_back(clauses.at(literal_id));
    }
  }
  current_operation_params.insert({operation_specific_params_string, params});
}

auto SQLQueryCreator::FlattenClauses(const std::string& current_process,
                                     int child_term_id, int current_term_id,
                                     int new_current_term_id) -> int {
  if (current_term_id == new_current_term_id) {
    filter_operations_relations_[current_process][current_term_id].insert(
        filter_operations_relations_[current_process][current_term_id].end(),
        filter_operations_relations_[current_process][child_term_id].begin(),
        filter_operations_relations_[current_process][child_term_id].end());
  } else {
    if (new_current_term_id == -1) {
      new_current_term_id = operation_counter_++;
      filter_operations_relations_[current_process].insert(
          {new_current_term_id, {}});
      filter_logic_[current_process].insert(
          {new_current_term_id,
           filter_logic_[current_process][current_term_id]});
    }
    filter_operations_relations_[current_process][new_current_term_id].insert(
        filter_operations_relations_[current_process][new_current_term_id]
            .end(),
        filter_operations_relations_[current_process][current_term_id].begin(),
        filter_operations_relations_[current_process][current_term_id].end());
    filter_operations_relations_[current_process][new_current_term_id].insert(
        filter_operations_relations_[current_process][new_current_term_id]
            .end(),
        filter_operations_relations_[current_process][child_term_id].begin(),
        filter_operations_relations_[current_process][child_term_id].end());
  }
  return new_current_term_id;
}

auto SQLQueryCreator::DistributeOrs(const std::string& current_process,
                                    int child_term_id, int current_term_id,
                                    int new_current_term_id) -> int {
  if (new_current_term_id == -1) {
    new_current_term_id = operation_counter_++;
    filter_operations_relations_[current_process].insert(
        {new_current_term_id, {}});
    filter_logic_[current_process].insert({new_current_term_id, false});

    if (IsLiteral(current_process, child_term_id)) {
      auto new_and_id = operation_counter_++;
      filter_operations_relations_[current_process].insert(
          {new_and_id, {child_term_id}});
      filter_logic_[current_process].insert({new_and_id, true});
      filter_operations_relations_[current_process][new_current_term_id]
          .push_back(new_and_id);
    } else {
      for (const auto id :
           filter_operations_relations_[current_process][child_term_id]) {
        if (IsLiteral(current_process, id)) {
          auto new_and_id = operation_counter_++;
          filter_operations_relations_[current_process].insert(
              {new_and_id, {id}});
          filter_logic_[current_process].insert({new_and_id, true});
          filter_operations_relations_[current_process][new_current_term_id]
              .push_back(new_and_id);
        } else {
          // TODO: Check that this is already an AND!
          filter_operations_relations_[current_process][new_current_term_id]
              .push_back(id);
        }
      }
    }
  } else {
    if (IsLiteral(current_process, child_term_id)) {
      for (const auto and_id :
           filter_operations_relations_[current_process][new_current_term_id]) {
        filter_operations_relations_[current_process][and_id].push_back(
            child_term_id);
      }
    } else {
      auto original_list =
          filter_operations_relations_[current_process][new_current_term_id];
      std::vector<int> new_list;
      for (const auto& new_literal_id :
           filter_operations_relations_[current_process][child_term_id]) {
        for (const auto& old_and_clause_id : original_list) {
          auto old_and_clause =
              filter_operations_relations_[current_process][old_and_clause_id];
          if (IsLiteral(current_process, new_literal_id)) {
            old_and_clause.push_back(new_literal_id);
          } else {
            // TODO: Check that this is already an AND!
            old_and_clause.insert(
                old_and_clause.end(),
                filter_operations_relations_[current_process][new_literal_id]
                    .begin(),
                filter_operations_relations_[current_process][new_literal_id]
                    .end());
          }
          auto new_and_id = operation_counter_++;
          filter_operations_relations_[current_process].insert(
              {new_and_id, {old_and_clause}});
          filter_logic_[current_process].insert({new_and_id, true});
          new_list.push_back(new_and_id);
        }
      }
      filter_operations_relations_[current_process][new_current_term_id] =
          new_list;
    }
  }
  return new_current_term_id;
}

auto SQLQueryCreator::IsLiteral(const std::string& current_process, int term_id)
    -> bool {
  return filter_operations_[current_process].find(term_id) ==
         filter_operations_[current_process].end();
}

auto SQLQueryCreator::TransofrmToDNF(const std::string& current_process,
                                     int current_term_id) -> int {
  // Example on how nested clauses get transformed:

  // And -> Or -> Or -> And -> Or
  // And -> Or -> Or -> Or -> And
  // And -> Or -> Or -> And
  // And -> Or -> And
  // Or -> And -> And
  // OR -> And

  if (IsLiteral(current_process, current_term_id)) {
    return current_term_id;
  } else {
    bool current_clause_is_and =
        filter_logic_[current_process][current_term_id];
    auto initial_child_clauses =
        filter_operations_relations_[current_process][current_term_id];
    int new_current_term_id = -1;
    bool need_to_distribute_ors = false;
    for (const auto& child_term_id : initial_child_clauses) {
      auto new_child_term_id = TransofrmToDNF(current_process, child_term_id);
      if (filter_logic_[current_process][child_term_id] ==
          current_clause_is_and) {
        new_current_term_id =
            FlattenClauses(current_process, child_term_id, current_term_id,
                           new_current_term_id);
      } else if (filter_logic_[current_process][child_term_id]) {
        // Is And - This is fine
      } else {
        need_to_distribute_ors = true;
      }
    }

    if (new_current_term_id != -1) {
      current_term_id = new_current_term_id;
      new_current_term_id = -1;
    }

    if (need_to_distribute_ors) {
      auto initial_child_clauses =
          filter_operations_relations_[current_process][current_term_id];
      for (const auto& child_term_id : initial_child_clauses) {
        new_current_term_id =
            DistributeOrs(current_process, child_term_id, current_term_id,
                          new_current_term_id);
      }
      return new_current_term_id;
    } else {
      return current_term_id;
    }
  }
}

void SQLQueryCreator::SetMultiplicationStreamParams(
    const std::string& current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  if (operations_[current_process].column_locations.find(
          operations_[current_process].operation_columns[0]) ==
          operations_[current_process].column_locations.end() ||
      operations_[current_process].column_locations.find(
          operations_[current_process].operation_columns[1]) ==
          operations_[current_process].column_locations.end()) {
    throw std::runtime_error("Multiplication column not found!");
  }
  auto first_column_location =
      operations_[current_process]
          .column_locations[operations_[current_process].operation_columns[0]];
  auto second_column_location =
      operations_[current_process]
          .column_locations[operations_[current_process].operation_columns[1]];
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
  current_operation_params.insert({operation_specific_params_string, {params}});
}
void SQLQueryCreator::SetAggregationStreamParams(
    const std::string& current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  if (operations_[current_process].column_locations.find(
          operations_[current_process].operation_columns[0]) ==
      operations_[current_process].column_locations.end()) {
    throw std::runtime_error("Aggregation column not found!");
  }
  auto column_location =
      operations_[current_process]
          .column_locations[operations_[current_process].operation_columns[0]];
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
}
void SQLQueryCreator::SetAdditionStreamParams(
    const std::string& current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  if (operations_[current_process].column_locations.find(
          operations_[current_process].operation_columns[0]) ==
      operations_[current_process].column_locations.end()) {
    throw std::runtime_error("Addition column not found!");
  }
  auto column_location =
      operations_[current_process]
          .column_locations[operations_[current_process].operation_columns[0]];
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
                  operations_.at(current_process).desired_columns.begin() +
          2);*/
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
  std::unordered_map<int, std::vector<int>> base_relations_map = {{0, {}}};
  filter_operations_relations_.insert({new_name, base_relations_map});
  std::unordered_map<int, bool> base_clause_map = {{0, false}};
  filter_logic_.insert({new_name, base_clause_map});
  // No literals initialised
  filter_operations_.insert({new_name, {}});
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

  FilterOperation new_operation;
  new_operation.column_name = column_name;
  new_operation.operation = comparison_type;
  new_operation.comparison_values =
      std::make_pair(compare_value, columns_.at(column_name).second);

  filter_operations_[filter_id].insert({operation_counter_, new_operation});

  return operation_counter_++;
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

  FilterOperation new_operation;
  new_operation.column_name = column_name;
  new_operation.operation = comparison_type;
  // TODO: Need checks that the format is correct!
  std::vector<int> comparison_values = {year * 10000 + month * 100 + day};
  new_operation.comparison_values = comparison_values;

  filter_operations_[filter_id].insert({operation_counter_, new_operation});

  return operation_counter_++;
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

  FilterOperation new_operation;
  new_operation.column_name = column_name;
  new_operation.operation = comparison_type;
  std::vector<int> comparison_values = {compare_value};
  new_operation.comparison_values = comparison_values;

  filter_operations_[filter_id].insert({operation_counter_, new_operation});

  return operation_counter_++;
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

  FilterOperation new_operation;
  new_operation.column_name = column_name;
  new_operation.operation = comparison_type;
  std::vector<int> comparison_values = {0,
                                        static_cast<int>(compare_value * 100)};
  new_operation.comparison_values = comparison_values;

  filter_operations_[filter_id].insert({operation_counter_, new_operation});

  return operation_counter_++;
}

auto SQLQueryCreator::AddOr(std::string filter_id,
                            std::vector<int> comparison_ids) -> int {
  filter_operations_relations_[filter_id].insert(
      {operation_counter_, comparison_ids});
  filter_logic_[filter_id].insert({operation_counter_, false});
  return operation_counter_++;
}

auto SQLQueryCreator::AddAnd(std::string filter_id,
                             std::vector<int> comparison_ids) -> int {
  filter_operations_relations_[filter_id].insert(
      {operation_counter_, comparison_ids});
  filter_logic_[filter_id].insert({operation_counter_, true});
  return operation_counter_++;
}

auto SQLQueryCreator::AddNot(std::string filter_id,
                             std::vector<int> comparison_ids) -> int {
  // For each AND or OR Claus when you hold a literal you have to say if it is
  // negative or positive.
  throw std::runtime_error("Not implemented yet.");
}