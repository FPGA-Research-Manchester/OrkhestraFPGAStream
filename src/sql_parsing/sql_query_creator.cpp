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
#include <limits>
#include <set>
#include <stdexcept>
#include <utility>
#include <iostream>

#include "logger.hpp"
#include "sql_json_writer.hpp"

using orkhestrafs::dbmstodspi::logging::Log;
using orkhestrafs::dbmstodspi::logging::LogLevel;
using orkhestrafs::sql_parsing::SQLJSONWriter;
using orkhestrafs::sql_parsing::SQLQueryCreator;

auto SQLQueryCreator::ExportInputDef() -> std::string {
  // TODO: Hardcoded for now
  const std::string graph_file = "intermediate_execution_graph.json";
  UpdateRequiredColumns();

  // Initialise starting points given input operations
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
  // TODO(Kaspar): Add crossbar reduction to output nodes - Add all nodes before
  // merge sort as output nodes as well!
  Log(LogLevel::kDebug, "Writing execution plan to: " + graph_file);

  SQLJSONWriter::WriteQuery(graph_file, data_to_write);
  Log(LogLevel::kInfo, "Finished parsing query. Plan stored to: " + graph_file);
  return graph_file;
  // return "benchmark_Q19_SF001.json";
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
        {operation_string_,
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
  // Should just be for Join.
  CombineOutputStreamParams(current_process, record_size);
  // TODO(Kaspar): Remove magic numbers!
  operations_[current_process].output_params[chunk_count_offset_].push_back(
      (record_size + 15) / 16);

  std::map<std::string, OperationParams> current_operation_params;
  OperationParams params;
  for (const auto& param_vector : operations_[current_process].input_params) {
    params.push_back(param_vector);
  }
  current_operation_params.insert({input_parameters_string_, params});
  params.clear();
  for (const auto& param_vector : operations_[current_process].output_params) {
    params.push_back(param_vector);
  }
  current_operation_params.insert({output_parameters_string_, params});

  SetOperationSpecificStreamParamsForDataMap(current_process,
                                             current_operation_params);
  current_parameters.insert(
      {operation_parameters_string_, current_operation_params});
}

void SQLQueryCreator::SetOperationSpecificStreamParamsForDataMap(
    const std::string& current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  // TODO(Kaspar): Currently set up to only do one column at a time.
  auto current_operation = operations_[current_process].operation_type;
  if (current_operation == QueryOperationType::kAddition) {
    SetAdditionStreamParams(current_process, current_operation_params);
  } else if (current_operation == QueryOperationType::kAggregationSum) {
    SetAggregationStreamParams(current_process, current_operation_params);
  } else if (current_operation == QueryOperationType::kMultiplication) {
    SetMultiplicationStreamParams(current_process, current_operation_params);
  } else if (current_operation == QueryOperationType::kFilter) {
    SetFilterStreamParams(current_process, current_operation_params);
  } else if (current_operation == QueryOperationType::kJoin) {
    SetJoinStreamParams(current_process, current_operation_params);
  } else {
    current_operation_params.insert({operation_specific_params_string_, {}});
  }
}

void SQLQueryCreator::SetJoinStreamParams(
    const std::string& current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  std::vector<int> join_params = {join_offset_};
  current_operation_params.insert(
      {operation_specific_params_string_, {join_params}});
}

void SQLQueryCreator::SetFilterStreamParams(
    const std::string& current_process,
    std::map<std::string, OperationParams>& current_operation_params) {
  int new_base_clause_id = TransformToDnf(current_process, 0);
  /*auto& current_filter_operations =
      filter_operations_relations_[current_process];*/

  std::set<std::vector<int>> all_clauses;

  /*auto clauses_after_base =
      filter_operations_relations_[current_process][new_base_clause_id];*/

  for (const auto& child_clause_id :
       filter_operations_relations_[current_process][new_base_clause_id]) {
    if (IsLiteral(current_process, child_clause_id)) {
      all_clauses.insert({child_clause_id});
    } else {
      auto new_clause =
          filter_operations_relations_[current_process][child_clause_id];
      std::sort(new_clause.begin(), new_clause.end());
      // For check
      /*for (const auto literal_test : new_clause) {
        if (filter_operations_relations_[current_process].find(literal_test) !=
            filter_operations_relations_[current_process].end()) {
          throw std::runtime_error("Transform was unsuccessful!");
        }
      }*/
      all_clauses.insert(new_clause);
    }
  }

  // TODO: Hardcoded limit. Should be checked according to available library and
  // if drivers support incomplete operation.
  if (all_clauses.size() > 32) {
    throw std::runtime_error(
        "Current system can't handle filter requirements with more than 32 DNF "
        "clauses!");
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
    }
    clauses.insert({literal_id, current_clauses});
  }

  OperationParams params;
  // TODO(Kaspar): Currently hardcoding decimal comparison and ignoring negation
  for (const auto& [column_name, literal_ids] : operations_on_a_column) {
    auto location = operations_[current_process].column_locations[column_name];
    std::vector<CompareFunctions> function_vector;
    std::vector<std::variant<std::pair<std::string, int>, int>>
        comparison_values;
    std::vector<std::vector<int>> clause_types;
    std::vector<std::vector<int>> clause_ids;
    if (std::holds_alternative<std::vector<int>>(
            filter_operations_[current_process][literal_ids.front()]
                .comparison_values) &&
        std::get<std::vector<int>>(
            filter_operations_[current_process][literal_ids.front()]
                .comparison_values)
                .size() != 1) {
      // Decimal comparison.
      for (const auto& literal_id : literal_ids) {
        function_vector.push_back(
            filter_operations_[current_process][literal_id].operation);
        comparison_values.emplace_back(
            std::get<std::vector<int>>(
                filter_operations_[current_process][literal_id]
                    .comparison_values)
                .at(1));
        clause_types.emplace_back();
        clause_ids.push_back(clauses.at(literal_id));
      }
      AddColumnFilteringParams(current_process, location + 1, function_vector,
                               comparison_values, clause_types, clause_ids,
                               params);
      function_vector = {CompareFunctions::kEqual};
      comparison_values = {0};
      clause_types = {{}};
      std::set<int> new_clause_ids_set;
      for (const auto& clause_id_vector : clause_ids) {
        for (const auto& id : clause_id_vector) {
          new_clause_ids_set.insert(id);
        }
      }
      std::vector<int> new_clause_ids(new_clause_ids_set.begin(),
                                      new_clause_ids_set.end());
      clause_ids = {new_clause_ids};
      AddColumnFilteringParams(current_process, location, function_vector,
                               comparison_values, clause_types, clause_ids,
                               params);

    } else {
      for (const auto& literal_id : literal_ids) {
        function_vector.push_back(
            filter_operations_[current_process][literal_id].operation);
        if (std::holds_alternative<std::pair<std::string, int>>(
                filter_operations_[current_process][literal_id]
                    .comparison_values)) {
          comparison_values.emplace_back(std::get<std::pair<std::string, int>>(
              filter_operations_[current_process][literal_id]
                  .comparison_values));
        } else {
          comparison_values.emplace_back(
              std::get<std::vector<int>>(
                  filter_operations_[current_process][literal_id]
                      .comparison_values)
                  .front());
        }
        clause_types.emplace_back();
        clause_ids.push_back(clauses.at(literal_id));
      }

      AddColumnFilteringParams(current_process, location, function_vector,
                               comparison_values, clause_types, clause_ids,
                               params);
    }
  }
  current_operation_params.insert({operation_specific_params_string_, params});
}

void SQLQueryCreator::AddColumnFilteringParams(
    const std::string& /*current_process*/, int location,
    const std::vector<CompareFunctions>& operations,
    const std::vector<std::variant<std::pair<std::string, int>, int>>&
        comparison_values,
    const std::vector<std::vector<int>>& clause_types,
    const std::vector<std::vector<int>>& clauses,
    OperationParams& resulting_params) {
  int operation_count = operations.size();

  // TODO(Kaspar): Get rid of the bunch of arrays and make a single vector of
  // structs!
  if (comparison_values.size() != operation_count ||
      clause_types.size() != operation_count ||
      clauses.size() != operation_count) {
    throw std::runtime_error(
        "Incorrect parameters given for filtering param filling!");
  }

  std::vector<int> location_vector;
  location_vector.push_back(location / 16);
  location_vector.push_back(15 - (location % 16));
  location_vector.push_back(operation_count);
  resulting_params.push_back(location_vector);

  for (int i = 0; i < operation_count; i++) {
    std::vector<int> function_vector;
    function_vector.push_back(compare_function_mapping_.at(operations.at(i)));
    resulting_params.push_back(function_vector);
    if (std::holds_alternative<std::pair<std::string, int>>(
            comparison_values.at(i))) {
      resulting_params.push_back(
          std::get<std::pair<std::string, int>>(comparison_values.at(i)));
    } else {
      auto current_value = std::get<int>(comparison_values.at(i));
      std::vector<int> value_vector = {current_value};
      resulting_params.push_back(value_vector);
    }
    resulting_params.push_back(clause_types.at(i));
    resulting_params.push_back(clauses.at(i));
  }
}

auto SQLQueryCreator::MakeANewClause(
    const std::string& current_process,
    const std::vector<int>& initial_child_clauses, bool is_and) -> int {
  filter_operations_relations_[current_process].insert(
      {operation_counter_, initial_child_clauses});
  is_and_[current_process].insert({operation_counter_, is_and});
  return operation_counter_++;
}

auto SQLQueryCreator::MakeACopyClause(const std::string& current_process,
                                      int original_term_id) -> int {
  return MakeANewClause(
      current_process,
      filter_operations_relations_[current_process][original_term_id],
      is_and_[current_process][original_term_id]);
}

auto SQLQueryCreator::FlattenClauses(const std::string& current_process,
                                     int child_term_id, int current_term_id,
                                     int new_current_term_id) -> int {
  if (new_current_term_id == -1) {
    new_current_term_id = MakeACopyClause(current_process, current_term_id);
  }
  RemoveFromClause(current_process, new_current_term_id, child_term_id);
  filter_operations_relations_[current_process][new_current_term_id].insert(
      filter_operations_relations_[current_process][new_current_term_id].end(),
      filter_operations_relations_[current_process][child_term_id].begin(),
      filter_operations_relations_[current_process][child_term_id].end());
  return new_current_term_id;
}

auto SQLQueryCreator::DistributeOrs(const std::string& current_process,
                                    int child_term_id, int /*current_term_id*/,
                                    int new_current_term_id) -> int {
  if (new_current_term_id == -1) {
    new_current_term_id = MakeANewClause(current_process, {}, false);
    if (IsLiteral(current_process, child_term_id)) {
      auto new_and_id = MakeANewClause(current_process, {child_term_id}, true);
      filter_operations_relations_[current_process][new_current_term_id]
          .push_back(new_and_id);
    } else {
      for (const auto id :
           filter_operations_relations_[current_process][child_term_id]) {
        if (IsLiteral(current_process, id)) {
          auto new_and_id = MakeANewClause(current_process, {id}, true);
          filter_operations_relations_[current_process][new_current_term_id]
              .push_back(new_and_id);
        } else {
          // TODO(Kaspar): Check that this is already an AND!
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
            // TODO(Kaspar): Check that this is already an AND!
            old_and_clause.insert(
                old_and_clause.end(),
                filter_operations_relations_[current_process][new_literal_id]
                    .begin(),
                filter_operations_relations_[current_process][new_literal_id]
                    .end());
          }

          auto new_and_id =
              MakeANewClause(current_process, {old_and_clause}, true);
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
  return filter_operations_[current_process].find(term_id) !=
         filter_operations_[current_process].end();
}

auto SQLQueryCreator::TransformToDnf(const std::string& current_process,
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
  }
  bool current_clause_is_and = is_and_[current_process][current_term_id];
  auto initial_child_clauses =
      filter_operations_relations_[current_process][current_term_id];
  int new_current_term_id = -1;
  bool need_to_distribute_ors = false;
  for (const auto& child_term_id : initial_child_clauses) {
    auto new_child_term_id = TransformToDnf(current_process, child_term_id);
    if (!IsLiteral(current_process, new_child_term_id)) {
      // Did it get transformed?
      if (new_child_term_id != child_term_id) {
        // Has current term been altered yet?
        if (new_current_term_id == -1) {
          new_current_term_id =
              MakeACopyClause(current_process, current_term_id);
        }
        // Remove original
        RemoveFromClause(current_process, new_current_term_id, child_term_id);
        // Add in new
        filter_operations_relations_[current_process][new_current_term_id]
            .push_back(new_child_term_id);
      }
      if (is_and_[current_process][new_child_term_id] ==
          current_clause_is_and) {
        new_current_term_id =
            FlattenClauses(current_process, new_child_term_id, current_term_id,
                           new_current_term_id);
      } else {
        // Either OR (AND) or AND (OR)
        if (!is_and_[current_process][new_child_term_id]) {
          // OR inside AND
          need_to_distribute_ors = true;
        }
      }
    }
  }

  if (new_current_term_id != -1) {
    current_term_id = new_current_term_id;
    new_current_term_id = -1;
  }

  if (need_to_distribute_ors) {
    initial_child_clauses =
        filter_operations_relations_[current_process][current_term_id];
    for (const auto& child_term_id : initial_child_clauses) {
      new_current_term_id = DistributeOrs(current_process, child_term_id,
                                          current_term_id, new_current_term_id);
    }

    // bool is_and = is_and_[current_process][new_current_term_id];
    //// If AND can throw an error if has any more ORs under it.
    // std::vector<int> child_ids =
    //    filter_operations_relations_[current_process][new_current_term_id];
    // std::vector<std::string>
    //        childs;
    // for (const auto& id: child_ids) {
    //  if (IsLiteral(current_process, id)) {
    //    childs.push_back("LITERAL");
    //  } else if (is_and_[current_process][id]) {
    //    childs.push_back("AND");
    //  } else {
    //      // Can throw an error here
    //    childs.push_back("OR");
    //  }
    //}

    return new_current_term_id;
  } else {
    /*bool is_and = is_and_[current_process][current_term_id];
    std::vector<int> child_ids =
        filter_operations_relations_[current_process][current_term_id];
    std::vector<std::string> childs;
    for (const auto& id : child_ids) {
      if (IsLiteral(current_process, id)) {
        childs.push_back("LITERAL");
      } else if (is_and_[current_process][id]) {
        childs.push_back("AND");
      } else {
        childs.push_back("OR");
      }
    }*/

    return current_term_id;
  }
}
void SQLQueryCreator::RemoveFromClause(const std::string& current_process,
                                       int parent_term_id,
                                       int removable_term_id) {
  filter_operations_relations_[current_process][parent_term_id].erase(
      std::remove(
          filter_operations_relations_[current_process][parent_term_id].begin(),
          filter_operations_relations_[current_process][parent_term_id].end(),
          removable_term_id),
      filter_operations_relations_[current_process][parent_term_id].end());
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
  //  if (first_column_location / 16 != second_column_location / 16 ||
  //      (first_column_location % 4) || (second_column_location % 2) ||
  //      (first_column_location + 2 != second_column_location)) {
  //    throw std::runtime_error("Multiplication columns incorrectly placed!");
  //  }
  std::vector<int> params;
  // TODO(Kaspar): Hardcoded to overwrite the first column at the moment!
  // Overwriting is done with hardcoded renaming.
  //  operations_[current_process].column_locations.erase(
  //      operations_[current_process].operation_columns[0]);
  //  operations_[current_process].column_locations.insert(
  //      {operations_[current_process].operation_columns[1],
  //       first_column_location});
  params.push_back(first_column_location / 16);
  for (int i = 0; i < 8; i++) {
    if (i == (first_column_location % 16) / 2) {
      params.push_back(1);
    } else {
      params.push_back(0);
    }
  }
  current_operation_params.insert(
      {operation_specific_params_string_, {params}});
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
  // TODO(Kaspar): Not sure if it has to be in range 16 or 8.
  std::vector<int> position_vector = {(column_location % 16) / 2};
  params.push_back(position_vector);
  current_operation_params.insert({operation_specific_params_string_, params});
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
  current_operation_params.insert({operation_specific_params_string_, params});
}

auto SQLQueryCreator::SetIOStreamParams(const std::string& current_process)
    -> int {
  // TODO(Kaspar): Still with multiple outputs the current approach doesn't
  // work.
  int record_size = 0;
  for (int parent_index = 0;
       parent_index < operations_.at(current_process).inputs.size();
       parent_index++) {
    if (parent_index == 0 &&
        !operations_[current_process].input_params.empty()) {
      // Skip the already processed stream
      auto column_types =
          operations_[current_process]
              .input_params[parent_index * io_param_vector_count_ +
                            data_types_offset_];
      auto column_sizes =
          operations_[current_process]
              .input_params[parent_index * io_param_vector_count_ +
                            data_sizes_offset_];
      for (int i = 0; i < column_types.size(); i++) {
        record_size +=
            column_sizes.at(i) *
            column_sizes_.at(static_cast<ColumnDataType>(column_types.at(i)));
      }
    } else {
      auto parent = operations_.at(current_process).inputs.at(parent_index);
      operations_[current_process].input_params.insert(
          operations_[current_process].input_params.end(), {{}, {}, {}, {}});
      operations_[current_process].output_params.insert(
          operations_[current_process].output_params.end(), {{}, {}, {}, {}});
      if (is_table_.at(parent)) {
        int last_used_column_index =
            ProcessTableColumns(current_process, parent_index, parent);

        record_size += PlaceColumnsToDesiredPositions(
            current_process, parent_index, last_used_column_index, parent,
            record_size);
        if (generate_join_offset_ == current_process && parent_index == 0) {
          if (join_offset_ != -1) {
            throw std::runtime_error("Offset is already set!");
          }
          join_offset_ = record_size % 16;
        }
      } else {
        CopyOutputParamsOfParent(current_process, parent_index, parent,
                                 record_size);
        auto column_types =
            operations_[current_process]
                .input_params[parent_index * io_param_vector_count_ +
                              data_types_offset_];
        auto column_sizes =
            operations_[current_process]
                .input_params[parent_index * io_param_vector_count_ +
                              data_sizes_offset_];
        for (int i = 0; i < column_types.size(); i++) {
          record_size +=
              column_sizes.at(i) *
              column_sizes_.at(static_cast<ColumnDataType>(column_types.at(i)));
        }
      }
    }
  }
  return record_size;
}

auto SQLQueryCreator::MapColumnPositions(
    const std::string& current_process, int stream_index,
    int last_needed_column_index, const std::string& table_name,
    std::map<std::string, std::vector<int>>& column_positions) -> int {
  int current_crossbar_index = 0;
  auto current_data_types_vector =
      operations_.at(current_process)
          .output_params.at(stream_index * io_param_vector_count_ +
                            data_types_offset_);
  for (int column_index = 0; column_index <= last_needed_column_index;
       column_index++) {
    auto current_column_name = tables_.at(table_name).at(column_index);
    if (column_renaming_map_.find(current_column_name) !=
        column_renaming_map_.end()) {
      current_column_name = column_renaming_map_.at(current_column_name);
    }
    auto [data_type, data_size] = columns_.at(current_column_name);
    int column_length = column_sizes_.at(data_type) * data_size;
    if (current_data_types_vector.at(column_index) !=
        static_cast<int>(ColumnDataType::kNull)) {
      std::vector<int> positions_vector;
      positions_vector.reserve(column_length);
      for (int i = 0; i < column_length; i++) {
        positions_vector.push_back(current_crossbar_index + i);
      }
      column_positions.insert({current_column_name, positions_vector});
    }
    current_crossbar_index += column_length;
  }
  return (current_crossbar_index + 15) / 16;
}

auto SQLQueryCreator::UsesMultipleChunks(std::vector<int> position_vector)
    -> bool {
  bool uses_multiple_chunks = false;
  // Assuming the position vector is not empty
  int first_position_chunk = position_vector.front() / 16;
  for (int i = 1; i < position_vector.size(); i++) {
    if (position_vector.at(i) / 16 != first_position_chunk) {
      uses_multiple_chunks = true;
    }
  }
  return uses_multiple_chunks;
}

void SQLQueryCreator::PlaceColumnsThatSpanOverMultipleChunks(
    std::vector<std::vector<int>>& crossbar_configuration,
    std::vector<std::string>& chosen_columns,
    const std::map<std::string, std::vector<int>>& column_positions,
    const std::string& current_process) {
  for (const auto& [column_name, position_vector] : column_positions) {
    if (UsesMultipleChunks(position_vector)) {
      // if it has position requirements - error
      if (operations_.at(current_process)
                  .desired_column_locations.find(column_name) !=
              operations_.at(current_process).desired_column_locations.end() ||
          operations_.at(current_process).paired_to_column.find(column_name) !=
              operations_.at(current_process).paired_to_column.end()) {
        throw std::runtime_error("Current column relocation isn't implemented");
      }
      for (int i : position_vector) {
        chosen_columns[i] = column_name;
        crossbar_configuration[i / 16][i % 16] = i;
      }
      operations_.at(current_process).column_locations[column_name] =
          position_vector.front();
    }
  }
}

// Column positions says which locations are the columns at currently
void SQLQueryCreator::GetCurrentAvailableDesiredPositions(
    const std::map<std::string, std::vector<int>>& column_positions,
    const std::string& current_process,
    std::map<int, std::vector<std::string>>& current_available_desired_columns,
    std::map<std::string, std::vector<int>>& left_over_availability) {
  for (const auto& [column_name, required_positions] :
       operations_.at(current_process).desired_column_locations) {
    if (column_positions.find(column_name) != column_positions.end()) {
      // TODO(Kaspar): Need to make this more foolproof!
      int current_chunk = column_positions.at(column_name).front() / 16;
      std::vector<int> translated_positions;
      for (const auto& chunk_independent_position : required_positions) {
        int translated_position =
            chunk_independent_position + current_chunk * 16;
        translated_positions.push_back(translated_position);
        if (current_available_desired_columns.find(translated_position) !=
            current_available_desired_columns.end()) {
          current_available_desired_columns[translated_position].push_back(
              column_name);
        } else {
          current_available_desired_columns.insert(
              {translated_position, {column_name}});
        }
      }
      current_chunk += 1;
      for (const auto& chunk_independent_position : required_positions) {
        int translated_position =
            chunk_independent_position + current_chunk * 16;
        translated_positions.push_back(translated_position);
        if (current_available_desired_columns.find(translated_position) !=
            current_available_desired_columns.end()) {
          current_available_desired_columns[translated_position].push_back(
              column_name);
        } else {
          current_available_desired_columns.insert(
              {translated_position, {column_name}});
        }
      }
      //current_chunk += 1;
      //for (const auto& chunk_independent_position : required_positions) {
      //  int translated_position =
      //      chunk_independent_position + current_chunk * 16;
      //  translated_positions.push_back(translated_position);
      //  if (current_available_desired_columns.find(translated_position) !=
      //      current_available_desired_columns.end()) {
      //    current_available_desired_columns[translated_position].push_back(
      //        column_name);
      //  } else {
      //    current_available_desired_columns.insert(
      //        {translated_position, {column_name}});
      //  }
      //}
      left_over_availability.insert({column_name, translated_positions});
    } else {
      // From some other table
    }
  }
}

void SQLQueryCreator::RemoveUnavailablePositions(
    std::map<int, std::vector<std::string>>& current_available_desired_columns,
    std::map<std::string, std::vector<int>>& left_over_availability,
    const std::vector<std::string>& chosen_columns,
    const std::map<std::string, std::vector<int>>& column_positions) {
  std::unordered_set<int> used_positions;
  const auto orig_copy = current_available_desired_columns;
  for (const auto& [location, desired_columns] : orig_copy) {
    for (const auto& column : desired_columns) {
      std::vector<int> position_vector;
      position_vector.reserve(column_positions.at(column).size());
      for (int i = 0; i < column_positions.at(column).size(); i++) {
        position_vector.push_back(location + i);
      }
      bool suitable = true;
      for (const auto& potential_position : position_vector) {
        if (!chosen_columns.at(potential_position).empty()) {
          suitable = false;
          break;
        }
      }
      // Find if any modulo in the same chunk is the same for any of the
      // potential positions.

      // 1. Get current columns modulos.
      // 2. Get current chunk columns
      // 3. Get the modulos of each and one of them.
      // 4. For all current modulos - check if any of them is in the set!
      
      // Hardcode 16 for now!
      if (suitable && column_positions.at(column).front() != location) {
        std::set<int> position_set;
        for (int i = 0; i < column_positions.at(column).size(); i++) {
          position_set.insert(column_positions.at(column).at(i) % 16);
        }
        int current_chunk_index = location / 16;
        std::set<std::string> current_columns;
        for (int i = current_chunk_index * 16;
             i < (current_chunk_index + 1) * 16 && i < chosen_columns.size();
             i++) {
          if (!chosen_columns.at(i).empty()) {
            current_columns.insert(chosen_columns.at(i));
          }
        }
        for (const auto& placed_column : current_columns) {
          if (!suitable) {
            break;
          }
          for (const auto& placed_column_loc :
               column_positions.at(placed_column)) {
            if (position_set.count(placed_column_loc % 16) > 0) {
              suitable = false;
              break;
            }
          }
        }
      }

      if (!suitable) {
        if (current_available_desired_columns.find(location) !=
            current_available_desired_columns.end()) {
          if (left_over_availability[column].empty()) {
            throw std::runtime_error(
                "Left over and desired columns out of sync!");
          }
          left_over_availability[column].erase(
              std::remove(left_over_availability[column].begin(),
                          left_over_availability[column].end(), location),
              left_over_availability[column].end());
          if (left_over_availability[column].empty()) {
            throw std::runtime_error("Can't place a column!");
          }
          current_available_desired_columns[location].erase(
              std::remove(current_available_desired_columns[location].begin(),
                          current_available_desired_columns[location].end(),
                          column),
              current_available_desired_columns[location].end());
          if (current_available_desired_columns[location].empty()) {
            current_available_desired_columns.erase(location);
          }
        }
      }
    }
  }

  // Can be deleted potentially
  /*for (int column_id = 0; column_id < chosen_columns.size(); column_id++) {
    if (!chosen_columns.at(column_id).empty()) {
      if (current_available_desired_columns.find(column_id) !=
          current_available_desired_columns.end()) {
        for (const auto& occupied_location_column_name :
             current_available_desired_columns.at(column_id)) {
          left_over_availability[occupied_location_column_name].erase(
              std::remove(
                  left_over_availability[occupied_location_column_name].begin(),
                  left_over_availability[occupied_location_column_name].end(),
                  column_id),
              left_over_availability[occupied_location_column_name].end());
          if (left_over_availability[occupied_location_column_name].empty()) {
            throw std::runtime_error("Can't place a column!");
          }
        }
        current_available_desired_columns.erase(column_id);
      }
    }
  }*/
}

// Actually not setting - just checking..
void SQLQueryCreator::SetJoinOffsetParam(const std::string& current_process,
                                         int offset, int stream_index) {
  // int join_offset = 0;
  // auto first_crossbar =
  //    operations_.at(current_process)
  //        .input_params.at(0 * io_param_vector_count_ + crossbar_offset_);
  // if (first_crossbar.empty()) {
  //  // Assuming the data sizes and types are collected correctly from the
  //  // previous operation.
  //  auto data_sizes =
  //      operations_.at(current_process)
  //          .input_params.at(0 * io_param_vector_count_ + data_sizes_offset_);
  //  auto data_types =
  //      operations_.at(current_process)
  //          .input_params.at(0 * io_param_vector_count_ + data_types_offset_);
  //  for (int i = 0; i < data_sizes.size(); i++) {
  //    join_offset +=
  //        data_sizes.at(i) *
  //        column_sizes_.at(static_cast<ColumnDataType>(data_types.at(i)));
  //  }
  //} else {
  //  join_offset = first_crossbar.size();
  //}
  // join_offset = join_offset % 16;
  // std::vector<int> offset_vector = {offset};
  // operations_.at(current_process).operation_params.push_back(offset_vector);

  // If you set the offset vector might as well check if it is possible.
  auto current_crossbar =
      operations_.at(current_process)
          .input_params.at(stream_index * io_param_vector_count_ +
                           crossbar_offset_);
  current_crossbar.resize(16, -1);
  int null_space_count = 0;
  for (const auto& column_target : current_crossbar) {
    if (column_target == -1) {
      null_space_count++;
    }
  }
  auto [sorted_by_data_type, sorted_by_data_size] =
      columns_.at(operations_.at(current_process).sorted_by_column);
  int sorted_by_column_length =
      column_sizes_.at(sorted_by_data_type) * sorted_by_data_size;
  null_space_count += sorted_by_column_length;
  if (null_space_count < offset) {
    throw std::runtime_error(
        "Join inter chunk data movement isn't implemented!");
  }
}

void SQLQueryCreator::RemoveAvailabliltyDueToJoinRequirements(
    std::map<int, std::vector<std::string>>& current_available_desired_columns,
    std::map<std::string, std::vector<int>>& left_over_availability,
    const std::string& current_process,
    std::vector<std::vector<int>>& crossbar_configuration,
    std::vector<std::string>& chosen_columns,
    const std::map<std::string, std::vector<int>>& column_positions,
    const std::map<std::string, std::string>& pairing_map, int offset,
    int stream_index) {
  SetJoinOffsetParam(current_process, offset, stream_index);
  // To set join key if missing.
  if (chosen_columns.front().empty()) {
    SetColumnPlace(current_available_desired_columns, left_over_availability,
                   crossbar_configuration, chosen_columns, column_positions,
                   operations_.at(current_process).sorted_by_column, 0,
                   pairing_map, current_process);
  }
  // To remove availability
  for (int i = 1; i < offset; i++) {
    if (current_available_desired_columns.find(i) !=
        current_available_desired_columns.end()) {
      SetColumnPlace(current_available_desired_columns, left_over_availability,
                     crossbar_configuration, chosen_columns, column_positions,
                     "", i, pairing_map, current_process);
    }
  }
}

void SQLQueryCreator::SetColumnPlace(
    std::map<int, std::vector<std::string>>& current_available_desired_columns,
    std::map<std::string, std::vector<int>>& left_over_availability,
    std::vector<std::vector<int>>& crossbar_configuration,
    std::vector<std::string>& chosen_columns,
    const std::map<std::string, std::vector<int>>& column_positions,
    const std::string& chosen_column_name, int chosen_location,
    const std::map<std::string, std::string>& pairing_map,
    const std::string& current_process) {

  // You can also use this method with "" to just clear postions.
  if (std::find(current_available_desired_columns.at(chosen_location).begin(),
                current_available_desired_columns.at(chosen_location).end(),
                chosen_column_name) ==
          current_available_desired_columns.at(chosen_location).end() &&
      !chosen_column_name.empty()) {
    throw std::runtime_error(
        "Can't insert this column to this location! (As it's not desired)");
  }

  if (!chosen_column_name.empty() &&
      pairing_map.find(chosen_column_name) != pairing_map.end()) {
    int paired_location =
        chosen_location + column_positions.at(chosen_column_name).size();
    const auto& paired_column = pairing_map.at(chosen_column_name);
    if (paired_location / 16 != chosen_location / 16) {
      throw std::runtime_error("Paired columns aren't in the same chunk!");
    }
    if (current_available_desired_columns.find(paired_location) !=
        current_available_desired_columns.end()) {
      current_available_desired_columns[paired_location].push_back(
          paired_column);
    } else {
      current_available_desired_columns.insert(
          {paired_location, {paired_column}});
    }
    if (left_over_availability.find(paired_column) !=
        left_over_availability.end()) {
      throw std::runtime_error(
          "Don't support paired columns with restrictions currently!");
    }
    left_over_availability.insert({paired_column, {paired_location}});

    SetColumnPlace(current_available_desired_columns, left_over_availability,
                   crossbar_configuration, chosen_columns, column_positions,
                   paired_column, paired_location, pairing_map,
                   current_process);
  }

  auto all_columns_allocated_for_current_location =
      current_available_desired_columns.at(chosen_location);

  for (const auto& current_location_candidate :
       all_columns_allocated_for_current_location) {
    if (chosen_column_name.empty()) {
      left_over_availability[current_location_candidate].erase(
          std::remove(
              left_over_availability[current_location_candidate].begin(),
              left_over_availability[current_location_candidate].end(),
              chosen_location),
          left_over_availability[current_location_candidate].end());
      if (left_over_availability[current_location_candidate].empty()) {
        throw std::runtime_error(
            "Used a position that was the only choice for another column!");
      }
      current_available_desired_columns.erase(chosen_location);
      continue;
    }
    std::vector<int> position_vector;
    position_vector.reserve(column_positions.at(chosen_column_name).size());
    for (int i = 0; i < column_positions.at(chosen_column_name).size(); i++) {
      position_vector.push_back(chosen_location + i);
    }
    if (current_location_candidate == chosen_column_name) {
      for (const auto& other_availablilites :
           left_over_availability.at(current_location_candidate)) {
        current_available_desired_columns[other_availablilites].erase(
            std::remove(
                current_available_desired_columns[other_availablilites].begin(),
                current_available_desired_columns[other_availablilites].end(),
                current_location_candidate),
            current_available_desired_columns[other_availablilites].end());
        if (current_available_desired_columns[other_availablilites].empty()) {
          current_available_desired_columns.erase(other_availablilites);
        }
      }
      left_over_availability.erase(chosen_column_name);
      if (UsesMultipleChunks(position_vector)) {
        throw std::runtime_error(
            "Can't place a column over multiple chunks at the moment!");
      }
      for (int i = 0; i < column_positions.at(chosen_column_name).size(); i++) {
        if (!chosen_columns[position_vector.at(i)].empty() ||
            crossbar_configuration[position_vector.at(i) / 16]
                                  [position_vector.at(i) % 16] != -1) {
          /*for (auto& choice : chosen_columns) {
            std::cout << choice << std::endl;
          }
          std::cout << position_vector.at(i) << std::endl;
          std::cout << chosen_columns[position_vector.at(i)] << std::endl;
          std::cout << chosen_column_name << std::endl;
          for (auto& value : column_positions) {
            std::cout << value.first << std::endl;
            for (auto& value1 : value.second) {
              std::cout << value1 << std::endl;
            }
          }*/
          throw std::runtime_error("This location isn't empty!:");
        }
        chosen_columns[position_vector.at(i)] = chosen_column_name;
        current_available_desired_columns.erase(position_vector.at(i));
        crossbar_configuration[position_vector.at(i) / 16]
                              [position_vector.at(i) % 16] =
                                  column_positions.at(chosen_column_name).at(i);
      }
      operations_.at(current_process).column_locations[chosen_column_name] =
          position_vector.front();
    } else {
      for (const auto& location : position_vector) {
        left_over_availability[current_location_candidate].erase(
            std::remove(
                left_over_availability[current_location_candidate].begin(),
                left_over_availability[current_location_candidate].end(),
                location),
            left_over_availability[current_location_candidate].end());
        if (left_over_availability[current_location_candidate].empty()) {
          throw std::runtime_error(
              "Used a position that was the only choice for another column!");
        }
        current_available_desired_columns[location].erase(
            std::remove(
                current_available_desired_columns[location].begin(),
                        current_available_desired_columns[location].end(),
                current_location_candidate),
            current_available_desired_columns[location].end());
        if (current_available_desired_columns[location].empty()) {
          current_available_desired_columns.erase(location);
        }
      }
    }
  }

  RemoveUnavailablePositions(current_available_desired_columns,
                             left_over_availability, chosen_columns,
                             column_positions);
}

void SQLQueryCreator::PlaceColumnsToPositionsWithOneAvailableLocation(
    std::map<int, std::vector<std::string>>& current_available_desired_columns,
    std::map<std::string, std::vector<int>>& left_over_availability,
    std::vector<std::vector<int>>& crossbar_configuration,
    std::vector<std::string>& chosen_columns,
    const std::map<std::string, std::vector<int>>& column_positions,
    const std::map<std::string, std::string>& pairing_map,
    const std::string& current_process) {
  std::set<int> positions_with_one_alternative;
  for (const auto& [position, desired_columns] :
       current_available_desired_columns) {
    if (desired_columns.size() == 1) {
      positions_with_one_alternative.insert(position);
    }
  }
  while (!positions_with_one_alternative.empty()) {
    auto next_postion_to_place = *positions_with_one_alternative.begin();
    positions_with_one_alternative.erase(
        positions_with_one_alternative.begin());

    if (current_available_desired_columns.at(next_postion_to_place).size() !=
        1) {
      throw std::runtime_error(
          "The current position doesn't have only one desired column!");
    }

    auto next_column_to_place =
        current_available_desired_columns.at(next_postion_to_place).front();
    for (const auto& other_location :
         left_over_availability.at(next_column_to_place)) {
      positions_with_one_alternative.erase(other_location);
    }

    for (const auto& location :
         left_over_availability.at(next_column_to_place)) {
      if (location != next_postion_to_place &&
          current_available_desired_columns.at(location).size() == 2) {
        positions_with_one_alternative.insert(location);
      }
    }

    SetColumnPlace(current_available_desired_columns, left_over_availability,
                   crossbar_configuration, chosen_columns, column_positions,
                   next_column_to_place, next_postion_to_place, pairing_map,
                   current_process);
  }
}

void SQLQueryCreator::PlaceGivenColumnsToGivenDesiredLocations(
    std::map<int, std::vector<std::string>>& current_available_desired_columns,
    std::map<std::string, std::vector<int>>& left_over_availability,
    std::vector<std::vector<int>>& crossbar_configuration,
    std::vector<std::string>& chosen_columns,
    const std::map<std::string, std::vector<int>>& column_positions,
    const std::map<std::string, std::string>& pairing_map,
    const std::string& current_process) {
  while (!current_available_desired_columns.empty()) {
    int min_alternative_count = std::numeric_limits<int>::max();
    for (const auto& [position, desired_columns] :
         current_available_desired_columns) {
      if (desired_columns.size() < min_alternative_count) {
        min_alternative_count = desired_columns.size();
      }
    }
    /*if (min_alternative_count == 1) {
      PlaceColumnsToPositionsWithOneAvailableLocation(
          current_available_desired_columns, left_over_availability,
          crossbar_configuration, chosen_columns, column_positions,
          pairing_map);
    } else {*/
    for (const auto& [position, desired_columns] :
         current_available_desired_columns) {
      if (desired_columns.size() == min_alternative_count) {
        int min_alternative_count_for_current_columns =
            std::numeric_limits<int>::max();
        std::string chosen_colum;
        for (const auto& current_column : desired_columns) {
          if (min_alternative_count_for_current_columns == 1 &&
              left_over_availability.at(current_column).size() == 1) {
            throw std::runtime_error(
                "Can't choose between mutliple columns that have the same "
                "only choice!");
          }
          if (left_over_availability.at(current_column).size() <
              min_alternative_count_for_current_columns) {
            min_alternative_count_for_current_columns =
                left_over_availability.at(current_column).size();
            chosen_colum = current_column;
          }
        }
        SetColumnPlace(current_available_desired_columns,
                       left_over_availability, crossbar_configuration,
                       chosen_columns, column_positions, chosen_colum, position,
                       pairing_map, current_process);
        break;
      }
    }
    /*}*/
  }
}

void SQLQueryCreator::CleanAvailablePositionsAndPlaceColumns(
    std::map<int, std::vector<std::string>>& current_available_desired_columns,
    std::map<std::string, std::vector<int>>& left_over_availability,
    const std::string& current_process,
    std::vector<std::vector<int>>& crossbar_configuration,
    std::vector<std::string>& chosen_columns,
    const std::map<std::string, std::vector<int>>& column_positions,
    int stream_index, const std::map<std::string, std::string>& pairing_map) {
  RemoveUnavailablePositions(current_available_desired_columns,
                             left_over_availability, chosen_columns, column_positions);

  if (required_join_offset_ == current_process &&
      (stream_index == 1 ||
       operations_.at(current_process).inputs.size() == 1)) {
    if (join_offset_ == -1) {
      throw std::runtime_error("Offset not generated!");
    }
    RemoveAvailabliltyDueToJoinRequirements(
        current_available_desired_columns, left_over_availability,
        current_process, crossbar_configuration, chosen_columns,
        column_positions, pairing_map, join_offset_, stream_index);
    // Don't need to do this again.
    //required_join_offset_ = "";
  }
  PlaceGivenColumnsToGivenDesiredLocations(
      current_available_desired_columns, left_over_availability,
      crossbar_configuration, chosen_columns, column_positions, pairing_map,
      current_process);
}

auto SQLQueryCreator::PlaceColumnsToDesiredPositions(
    const std::string& current_process, int stream_index,
    int last_needed_column_index, std::string table_name, int record_size)
    -> int {
  // TODO(Kaspar): Add a quick check that there are more positions than columns
  // with desired positions!

  std::map<std::string, std::string> reverse_pairing_map;
  for (const auto& [second_column, first_column] :
       operations_.at(current_process).paired_to_column) {
    if (reverse_pairing_map.find(first_column) != reverse_pairing_map.end()) {
      throw std::runtime_error(
          "Can't have a column paired to multiple columns!");
    }
    reverse_pairing_map.insert({first_column, second_column});
  }

  std::map<std::string, std::vector<int>> column_positions;
  auto chunk_count = MapColumnPositions(
      current_process, stream_index, last_needed_column_index,
      std::move(table_name), column_positions);
  std::vector<std::vector<int>> crossbar_configuration;
  std::vector<std::string> chosen_columns;
  std::vector<int> column_types;
  std::vector<int> column_sizes;
  // Non-optimal implementation for clarity
  for (int i = 0; i < chunk_count + 1; i++) {
    std::vector<int> chunk_crossbar_configuration(16, -1);
    crossbar_configuration.push_back(chunk_crossbar_configuration);
    for (int j = 0; j < 16; j++) {
      chosen_columns.emplace_back("");
    }
  }

  PlaceColumnsThatSpanOverMultipleChunks(crossbar_configuration, chosen_columns,
                                         column_positions, current_process);

  std::map<int, std::vector<std::string>> current_available_desired_columns;
  std::map<std::string, std::vector<int>> left_over_availability;

  // TODO(Kaspar): Add a check that the paired column has suitable positions
  for (const auto& [first_paired_column, second_paired_column] :
       reverse_pairing_map) {
    operations_.at(current_process)
        .desired_column_locations.erase(second_paired_column);
  }
  GetCurrentAvailableDesiredPositions(column_positions, current_process,
                                      current_available_desired_columns,
                                      left_over_availability);
  CleanAvailablePositionsAndPlaceColumns(
      current_available_desired_columns, left_over_availability,
      current_process, crossbar_configuration, chosen_columns, column_positions,
      stream_index, reverse_pairing_map);

  std::set<std::string> placed_columns;
  for (const auto& column_name : chosen_columns) {
    if (!column_name.empty()) {
      placed_columns.insert(column_name);
    }
  }
  for (const auto& [column_name, placed_locations] : column_positions) {
    if (placed_columns.find(column_name) == placed_columns.end() &&
        reverse_pairing_map.find(column_name) == reverse_pairing_map.end()) {
      // Just for clarity
      //int column_chunk = placed_locations.front() / 16;
      int column_length = placed_locations.size();

      std::vector<int> desired_locations;
      desired_locations.reserve(16 - column_length);
      for (int j = 0; j < chosen_columns.size() / 16; j++) {
        for (int i = 0; i < 16 - column_length; i++) {
          desired_locations.push_back(j * 16 + i);
        }
      }
      

      left_over_availability.insert({column_name, desired_locations});
      for (const auto& location : desired_locations) {
        if (current_available_desired_columns.find(location) !=
            current_available_desired_columns.end()) {
          current_available_desired_columns[location].push_back(column_name);
        } else {
          current_available_desired_columns.insert({location, {column_name}});
        }
      }
    }
  }
  CleanAvailablePositionsAndPlaceColumns(
      current_available_desired_columns, left_over_availability,
      current_process, crossbar_configuration, chosen_columns, column_positions,
      stream_index, reverse_pairing_map);

  // Duplicate data
  for (const auto& desired_column :
       operations_[current_process].desired_columns) {
    if (std::find(chosen_columns.begin(), chosen_columns.end(),
                  desired_column) == chosen_columns.end() &&
        original_of_an_duplicated_column_.find(desired_column) !=
            original_of_an_duplicated_column_.end() &&
        std::find(chosen_columns.begin(), chosen_columns.end(),
                  original_of_an_duplicated_column_.at(desired_column)) !=
            chosen_columns.end()) {
      auto& original_column =
          original_of_an_duplicated_column_.at(desired_column);
      if (columns_.find(desired_column) == columns_.end()) {
        columns_.insert({desired_column, columns_.at(original_column)});
      }
      auto original_chunk_index =
          operations_[current_process].column_locations.at(original_column) /
          16;
      auto new_chunk_index = original_chunk_index;
      auto required_column_word_size = static_cast<int>(
          columns_.at(original_column).second *
          column_sizes_.at(columns_.at(original_column).first));
      auto& original_chunk = crossbar_configuration.at(original_chunk_index);
      int good_location = -1;
      int current_match_length = 0;
      int start_i = 0;
      if (required_join_offset_ == current_process &&
          (stream_index == 1 ||
           operations_.at(current_process).inputs.size() == 1) && join_offset_ != -1) {
        start_i = join_offset_;
      }
      for (int i = start_i; i < original_chunk.size(); i++) {
        if (original_chunk.at(i) == -1) {
          if (good_location == -1) {
            good_location = i;
          }
          current_match_length++;
          if (current_match_length == required_column_word_size) {
            // location found!
            break;
          }
        } else {
          good_location = -1;
          current_match_length = 0;
        }
      }
      if (current_match_length != required_column_word_size) {
        good_location = -1;
      }
      if (good_location == -1) {
        // TODO: Move the original value up as well.
        new_chunk_index++;
        auto& new_chunk = crossbar_configuration.at(new_chunk_index);
        current_match_length = 0;
        start_i = 0;
        //if (required_join_offset_ == current_process &&
        //    (stream_index == 1 ||
        //     operations_.at(current_process).inputs.size() == 1)) {
        //  start_i = 1;
        //}
        for (int i = start_i; i < new_chunk.size(); i++) {
          if (new_chunk.at(i) == -1) {
            if (good_location == -1) {
              good_location = i;
            }
            current_match_length++;
            if (current_match_length == required_column_word_size) {
              // location found!
              break;
            }
          } else {
            good_location = -1;
            current_match_length = 0;
          }
        }
      }
      if (good_location == -1) {
        throw std::runtime_error(
            "Can't find an available location to duplicate data!");
      }
      else {
        // Set location
        operations_[current_process].column_locations[desired_column] =
            good_location + new_chunk_index * 16;
        int original_crossbar_location =
            operations_[current_process].column_locations.at(original_column) %
            16;
        // Copy over crossbar data and write in column names
        for (int i = 0; i < required_column_word_size; i++) {
          crossbar_configuration[new_chunk_index][good_location + i] =
              crossbar_configuration[original_chunk_index]
                            [original_crossbar_location + i];
          chosen_columns[operations_[current_process]
                             .column_locations[desired_column] +
                         i] = desired_column;
        }
      }
    }
  }

  // Set column types and column sizes
  std::string current_column_name = chosen_columns.front();
  int current_column_length = 0;
  for (const auto& column_name : chosen_columns) {
    record_size++;
    if (column_name == current_column_name) {
      current_column_length++;
    } else {
      if (!current_column_name.empty()) {
        auto column_type = columns_.at(current_column_name).first;
        column_sizes.push_back(current_column_length /
                               column_sizes_.at(column_type));
        column_types.push_back(static_cast<int>(column_type));

      } else {
        column_sizes.push_back(current_column_length);
        column_types.push_back(static_cast<int>(ColumnDataType::kNull));
      }
      current_column_length = 1;
      current_column_name = column_name;
    }
  }
  if (!current_column_name.empty()) {
    auto column_type = columns_.at(current_column_name).first;
    column_sizes.push_back(current_column_length /
                           column_sizes_.at(column_type));
    column_types.push_back(static_cast<int>(column_type));

  } else {
    record_size -= current_column_length;
  }

  // Flatten the crossbar
  std::vector<int> flattened_crossbar_config;
  for (const auto& chunk_crossbar_config : crossbar_configuration) {
    flattened_crossbar_config.insert(flattened_crossbar_config.end(),
                                     chunk_crossbar_config.begin(),
                                     chunk_crossbar_config.end());
  }
  flattened_crossbar_config.resize(record_size);

  if (operations_.at(current_process).operation_type ==
          QueryOperationType::kJoin &&
      stream_index == 1) {
    if (join_offset_ == -1) {
      throw std::runtime_error("Offset is not set!");
    }
    int offset = join_offset_;
    /*int offset = std::get<std::vector<int>>(
                     operations_.at(current_process).operation_params.front())
                     .front();
    record_size -= offset;

    int removed_positions_length = 0;
    while (removed_positions_length < offset) {
      removed_positions_length +=
          column_sizes_.at(static_cast<ColumnDataType>(column_types.front())) *
          column_sizes.front();
      column_sizes.erase(column_sizes.begin());
      column_types.erase(column_types.begin());
    }*/
  }

  operations_[current_process]
      .output_params[stream_index * io_param_vector_count_ +
                     data_types_offset_] = column_types;
  operations_[current_process]
      .output_params[stream_index * io_param_vector_count_ +
                     data_sizes_offset_] = column_sizes;
  operations_[current_process]
      .input_params[stream_index * io_param_vector_count_ + crossbar_offset_] =
      flattened_crossbar_config;

  return record_size;
}

void SQLQueryCreator::CombineOutputStreamParams(
    const std::string& current_process, int& record_size) {
  if (operations_.at(current_process).inputs.size() > 2) {
    throw std::runtime_error("Unsopported operation!");
  }
  for (int parent_index = 1;
       parent_index < operations_.at(current_process).inputs.size();
       parent_index++) {
    if (operations_[current_process]
            .output_params[parent_index * io_param_vector_count_ +
                           data_sizes_offset_]
            .size() > 1) {
      auto data_sizes =
          operations_[current_process]
              .output_params[parent_index * io_param_vector_count_ +
                             data_sizes_offset_];
      auto data_types =
          operations_[current_process]
              .output_params[parent_index * io_param_vector_count_ +
                             data_types_offset_];
      int number_of_discarded_columns = 2;
      int first_column_size =
          data_sizes.at(0) *
          column_sizes_.at(static_cast<ColumnDataType>(data_types.at(0)));
      int removed_columns_size = first_column_size;
      if (removed_columns_size == join_offset_) {
        number_of_discarded_columns = 1;
        record_size -= removed_columns_size;
      } else {
        int second_column_size =
            data_sizes.at(1) *
            column_sizes_.at(static_cast<ColumnDataType>(data_types.at(1)));
        removed_columns_size += second_column_size;

        if (removed_columns_size > join_offset_) {
          if (static_cast<ColumnDataType>(data_types.at(1)) ==
              ColumnDataType::kNull) {
            operations_[current_process]
                .output_params[data_types_offset_]
                .push_back(static_cast<int>(ColumnDataType::kNull));
            int null_size =
                second_column_size - (join_offset_ - first_column_size);
            operations_[current_process]
                .output_params[data_sizes_offset_]
                .push_back(null_size);
            record_size -= removed_columns_size;
            record_size += null_size;
          } else {
            throw std::runtime_error(
                "Can't NULL-ify non-NULL columns with join currently!");
          }
        } else if (removed_columns_size == join_offset_) {
          record_size -= removed_columns_size;
          // Everything is fine. The 2nd column will get removed as well.
        } else {
          // TODO: implement removing further columns!
          throw std::runtime_error("Don't support joins that remove columns!");
        }
      }

      for (int i = number_of_discarded_columns;
           i < operations_[current_process]
                   .output_params[parent_index * io_param_vector_count_ +
                                  data_sizes_offset_]
                   .size();
           i++) {
        operations_[current_process]
            .output_params[data_sizes_offset_]
            .push_back(
                operations_[current_process]
                    .output_params[parent_index * io_param_vector_count_ +
                                   data_sizes_offset_][i]);
        operations_[current_process]
            .output_params[data_types_offset_]
            .push_back(
                operations_[current_process]
                    .output_params[parent_index * io_param_vector_count_ +
                                   data_types_offset_][i]);
      }
    }
  }
  operations_[current_process].output_params.resize(io_param_vector_count_);
}

void SQLQueryCreator::CopyOutputParamsOfParent(
    const std::string& current_process, int parent_index,
    const std::string& parent, int record_size) {
  operations_[current_process]
      .input_params[parent_index * io_param_vector_count_ +
                    data_types_offset_] =
      operations_[parent].output_params[data_types_offset_];
  operations_[current_process]
      .input_params[parent_index * io_param_vector_count_ +
                    data_sizes_offset_] =
      operations_[parent].output_params[data_sizes_offset_];
  operations_[current_process]
      .output_params[parent_index * io_param_vector_count_ +
                     data_types_offset_] =
      operations_[parent].output_params[data_types_offset_];
  operations_[current_process]
      .output_params[parent_index * io_param_vector_count_ +
                     data_sizes_offset_] =
      operations_[parent].output_params[data_sizes_offset_];
  for (const auto& [column_name, column_location] :
       operations_[parent].column_locations) {
    if (parent_index != 0 &&
        column_name != operations_[current_process].sorted_by_column) {
      operations_[current_process].column_locations.insert(
          {column_name, column_location + record_size - join_offset_});
    } else {
      operations_[current_process].column_locations.insert(
          {column_name, column_location});
    }
  }
}

auto SQLQueryCreator::CompressNullColumns(const std::string& current_process,
                                          int stream_index,
                                          int used_column_count) -> int {
  int record_size = 0;
  std::vector<int> new_data_types_concat;
  std::vector<int> new_data_sizes_concat;
  int crossbar_config_length = 0;
  for (int column_index = 0; column_index <= used_column_count;
       column_index++) {
    auto data_size =
        operations_.at(current_process)
            .output_params
            .at(stream_index * io_param_vector_count_ + data_sizes_offset_)
            .at(column_index);
    auto data_type =
        operations_.at(current_process)
            .output_params
            .at(stream_index * io_param_vector_count_ + data_types_offset_)
            .at(column_index);
    int column_length =
        column_sizes_.at(static_cast<ColumnDataType>(data_type)) * data_size;
    record_size += column_length;
    if (!new_data_types_concat.empty() &&
        data_type == static_cast<int>(ColumnDataType::kNull) &&
        new_data_types_concat.back() ==
            static_cast<int>(ColumnDataType::kNull)) {
      new_data_sizes_concat.back() += data_size;
    } else {
      new_data_types_concat.push_back(data_type);
      new_data_sizes_concat.push_back(data_size);
    }
    crossbar_config_length += column_length;
  }
  operations_[current_process]
      .output_params[stream_index * io_param_vector_count_ +
                     data_sizes_offset_] = new_data_sizes_concat;
  operations_[current_process]
      .output_params[stream_index * io_param_vector_count_ +
                     data_types_offset_] = new_data_types_concat;
  operations_[current_process]
      .input_params[stream_index * io_param_vector_count_ + crossbar_offset_]
      .resize(crossbar_config_length);
  return record_size;
}
auto SQLQueryCreator::ProcessTableColumns(const std::string& current_process,
                                          int parent_index,
                                          const std::string& table_name)
    -> int {
  int current_position_index = 0;
  int last_used_column_index = 0;
  for (int column_index = 0; column_index < tables_.at(table_name).size();
       column_index++) {
    auto current_column_name = tables_.at(table_name).at(column_index);
    if (column_renaming_map_.find(current_column_name) !=
        column_renaming_map_.end()) {
      current_column_name = column_renaming_map_.at(current_column_name);
    }
    // Set original data types and sizes.
    auto [data_type, data_size] = columns_.at(current_column_name);
    operations_[current_process]
        .input_params[parent_index * io_param_vector_count_ +
                      data_types_offset_]
        .push_back(static_cast<int>(data_type));
    operations_[current_process]
        .input_params[parent_index * io_param_vector_count_ +
                      data_sizes_offset_]
        .push_back(data_size);
    int column_length = column_sizes_.at(data_type) * data_size;

    if (std::find(operations_.at(current_process).desired_columns.begin(),
                  operations_.at(current_process).desired_columns.end(),
                  current_column_name) !=
        operations_.at(current_process).desired_columns.end()) {
      operations_[current_process].column_locations.insert(
          {current_column_name, current_position_index});
      last_used_column_index = column_index;
      for (int i = 0; i < column_length; i++) {
        operations_[current_process]
            .input_params[parent_index * io_param_vector_count_ +
                          crossbar_offset_]
            .push_back(current_position_index++);
      }
      // Hardcoded 0 index to only support a single output.
      operations_[current_process]
          .output_params[parent_index * io_param_vector_count_ +
                         data_types_offset_]
          .push_back(static_cast<int>(data_type));
      operations_[current_process]
          .output_params[parent_index * io_param_vector_count_ +
                         data_sizes_offset_]
          .push_back(data_size);
    } else {
      for (int i = 0; i < column_length; i++) {
        operations_[current_process]
            .input_params[parent_index * io_param_vector_count_ +
                          crossbar_offset_]
            .push_back(-1);
        current_position_index++;
      }
      operations_[current_process]
          .output_params[parent_index * io_param_vector_count_ +
                         data_types_offset_]
          .push_back(static_cast<int>(ColumnDataType::kNull));
      operations_[current_process]
          .output_params[parent_index * io_param_vector_count_ +
                         data_sizes_offset_]
          .push_back(column_length);
    }
  }
  return last_used_column_index;
}
void SQLQueryCreator::SetOutputsForDataMap(
    const std::string& current_process, InputNodeParameters& current_parameters,
    std::unordered_set<std::string>& processed_operations,
    std::unordered_set<std::string>& operations_to_process) {
  // TODO(Kaspar): Breaks when a final node has multiple outputs!
  std::vector<std::string> output_files;
  std::vector<std::string> output_nodes;
  for (const auto& child : operations_.at(current_process).outputs) {
    // Currently, can't set output as a table
    output_files.emplace_back("");
    output_nodes.push_back(child);
    UpdateNextOperationsListIfAvailable(processed_operations,
                                        operations_to_process, child);
  }
  // Temporary workaround
  if (output_files.empty()) {
    output_files.emplace_back("");
    output_nodes.emplace_back("");
  }
  current_parameters.insert({output_files_string_, output_files});
  current_parameters.insert({output_nodes_string_, output_nodes});
}
void SQLQueryCreator::SetInputsForDataMap(
    const std::string& current_process,
    InputNodeParameters& current_parameters) {
  std::vector<std::string> input_files;
  std::vector<std::string> input_nodes;
  for (const auto& parent : operations_.at(current_process).inputs) {
    if (is_table_.at(parent)) {
      input_files.push_back(parent);
      input_nodes.emplace_back("");
    } else {
      input_nodes.push_back(parent);
      input_files.emplace_back("");
    }
  }
  current_parameters.insert({input_files_string_, input_files});
  current_parameters.insert({input_nodes_string_, input_nodes});
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
    // Pass down join requirements
    if (operations_.at(current_process).operation_type ==
        QueryOperationType::kJoin) {
      if (operations_.at(current_process).inputs.size() != 2) {
        throw std::runtime_error("Unsupported join!");
      }
      const auto& first_stream = operations_.at(current_process).inputs.at(0);
      if (!is_table_.at(first_stream)) {
        generate_join_offset_ = first_stream;
      } else {
        // Will get fixed later
      }
      const auto& second_stream = operations_.at(current_process).inputs.at(1);
      if (!is_table_.at(second_stream)) {
        required_join_offset_ = second_stream;
      }
    } else {
      if (operations_.at(current_process).inputs.size() != 1) {
        throw std::runtime_error("Unsupported multi-input operation!");
      }
      const auto& input = operations_.at(current_process).inputs.front();
      if (!is_table_.at(input)) {
        // Can't be both at once
        // TODO: add an error check!
        if (current_process == generate_join_offset_) {
          generate_join_offset_ = input;
        } else if (current_process == required_join_offset_) {
          required_join_offset_ = input;
        }
      }
    }
    // Uodate desired columns
    for (const auto& parent : operations_.at(current_process).inputs) {
      if (!is_table_.at(parent)) {
        for (const auto& column :
             operations_.at(current_process).desired_columns) {
          // Dealing with renamed columns after.
          if (std::find(operations_.at(parent).desired_columns.begin(),
                        operations_.at(parent).desired_columns.end(), column) ==
              operations_.at(parent).desired_columns.end()) {
            operations_.at(parent).desired_columns.push_back(column);
          }
          // TODO(Kaspar): Handle these without crashing!
          if (operations_.at(current_process)
                  .desired_column_locations.find(column) !=
              operations_.at(current_process).desired_column_locations.end()) {
            if (operations_.at(parent).desired_column_locations.find(column) !=
                    operations_.at(parent).desired_column_locations.end() &&
                operations_.at(parent).desired_column_locations.at(column) !=
                    operations_.at(current_process)
                        .desired_column_locations.at(column)) {
              throw std::runtime_error("Column location already specified!");
            }
            operations_.at(parent).desired_column_locations.insert(
                {column, operations_.at(current_process)
                             .desired_column_locations.at(column)});
          }
          if (operations_.at(current_process).paired_to_column.find(column) !=
              operations_.at(current_process).paired_to_column.end()) {
            if (operations_.at(parent).paired_to_column.find(column) !=
                    operations_.at(parent).paired_to_column.end() &&
                operations_.at(parent).paired_to_column.at(column) !=
                    operations_.at(current_process)
                        .paired_to_column.at(column)) {
              throw std::runtime_error("Column pairing already specified!");
            }
            operations_.at(parent).paired_to_column.insert(
                {column,
                 operations_.at(current_process).paired_to_column.at(column)});
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
  RenameAllColumns();

  if (!generate_join_offset_.empty() &&
      operations_.at(generate_join_offset_).operation_type ==
          QueryOperationType::kJoin) {
    // Need to know the size of the table after crossbar!
    auto parent = operations_.at(generate_join_offset_).inputs.at(0);
    operations_[generate_join_offset_].input_params.insert(
        operations_[generate_join_offset_].input_params.end(),
        {{}, {}, {}, {}});
    operations_[generate_join_offset_].output_params.insert(
        operations_[generate_join_offset_].output_params.end(),
        {{}, {}, {}, {}});
    int last_used_column_index =
        ProcessTableColumns(generate_join_offset_, 0, parent);

    int record_size = PlaceColumnsToDesiredPositions(
        generate_join_offset_, 0, last_used_column_index, parent, 0);
    join_offset_ = record_size % 16;
    generate_join_offset_ = "invalid_";
  }
}

void SQLQueryCreator::RenameAllColumns() {
  for (auto& [operation_key, operation_data] : operations_) {
    for (const auto& [renamed_column, new_name] : column_renaming_map_) {
      if (operation_data.sorted_by_column == renamed_column) {
        operation_data.sorted_by_column = new_name;
      }
      if (std::find(operation_data.desired_columns.begin(),
                    operation_data.desired_columns.end(),
                    renamed_column) != operation_data.desired_columns.end()) {
        operation_data.desired_columns.erase(
            std::remove(operation_data.desired_columns.begin(),
                        operation_data.desired_columns.end(), renamed_column),
            operation_data.desired_columns.end());
        if (std::find(operation_data.desired_columns.begin(),
                      operation_data.desired_columns.end(),
                      new_name) == operation_data.desired_columns.end()) {
          operation_data.desired_columns.push_back(new_name);
        }
      }
      // TODO(Kaspar): Refactor these 2 ifs.
      if (operation_data.desired_column_locations.find(renamed_column) !=
          operation_data.desired_column_locations.end()) {
        if (operation_data.desired_column_locations.find(new_name) !=
            operation_data.desired_column_locations.end()) {
          if (operation_data.desired_column_locations.at(new_name) !=
              operation_data.desired_column_locations.at(renamed_column)) {
            throw std::runtime_error(
                "Renamed columns have different specifications!");
          }
        } else {
          operation_data.desired_column_locations.insert(
              {new_name,
               operation_data.desired_column_locations.at(renamed_column)});
        }
        operation_data.desired_column_locations.erase(renamed_column);
      }
      if (operation_data.paired_to_column.find(renamed_column) !=
          operation_data.paired_to_column.end()) {
        if (operation_data.paired_to_column.find(new_name) !=
            operation_data.paired_to_column.end()) {
          if (operation_data.paired_to_column.at(new_name) !=
              operation_data.paired_to_column.at(renamed_column)) {
            throw std::runtime_error(
                "Renamed columns have different specifications!");
          }
        } else {
          operation_data.paired_to_column.insert(
              {new_name, operation_data.paired_to_column.at(renamed_column)});
        }
        operation_data.paired_to_column.erase(renamed_column);
      }
    }
    for (const auto& [key, value] : operation_data.paired_to_column) {
      if (column_renaming_map_.find(value) != column_renaming_map_.end()) {
        operation_data.paired_to_column[key] = column_renaming_map_.at(value);
      }
    }
  }

  for (const auto& [renamed_column, new_name] : column_renaming_map_) {
    columns_.insert({new_name, columns_.at(renamed_column)});
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
                                    int /*row_count*/) -> std::string {
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
  auto new_name =
      RegisterOperation(QueryOperationType::kFilter, {std::move(input)});
  std::unordered_map<int, std::vector<int>> base_relations_map = {{0, {}}};
  filter_operations_relations_.insert({new_name, base_relations_map});
  std::unordered_map<int, bool> base_clause_map = {{0, false}};
  is_and_.insert({new_name, base_clause_map});
  // No literals initialised
  filter_operations_.insert({new_name, {}});
  return new_name;
}

auto SQLQueryCreator::RegisterSort(std::string input, std::string column_name)
    -> std::string {
  // Check here if column has been renamed already!
  if (column_renaming_map_.find(column_name) != column_renaming_map_.end()) {
    column_name = column_renaming_map_.at(column_name);
  }

  if ((is_table_.at(input) && tables_.at(input).front() == column_name) ||
      (!is_table_.at(input) &&
       operations_.at(input).sorted_by_column == column_name)) {
    return input;
  }
  auto lin_sort_name =
      RegisterOperation(QueryOperationType::kLinearSort, {input});
  auto merge_sort_name =
      RegisterOperation(QueryOperationType::kMergeSort, {lin_sort_name});
  operations_[lin_sort_name].desired_columns.push_back(column_name);
  operations_[lin_sort_name].sorted_by_column = column_name;
  operations_[merge_sort_name].desired_columns.push_back(column_name);
  operations_[merge_sort_name].sorted_by_column = column_name;
  operations_[lin_sort_name].desired_column_locations.insert(
      {column_name, {0}});
  operations_[merge_sort_name].desired_column_locations.insert(
      {column_name, {0}});
  return merge_sort_name;
}

auto SQLQueryCreator::RegisterJoin(std::string first_input,
                                   const std::string& first_join_key,
                                   std::string second_input,
                                   const std::string& second_join_key)
    -> std::string {
  if (columns_.at(first_join_key) != columns_.at(second_join_key) &&
      column_sizes_.at(columns_.at(first_join_key).first) *
              columns_.at(first_join_key).second !=
          1) {
    throw std::runtime_error("Unsopported join keys!");
  }
  if (!generate_join_offset_.empty() || !required_join_offset_.empty()) {
    throw std::runtime_error("Currently we don't support more than 1 join");
  }

  auto first_join_input = RegisterSort(std::move(first_input), first_join_key);
  auto second_join_input =
      RegisterSort(std::move(second_input), second_join_key);
  auto join_name = RegisterOperation(QueryOperationType::kJoin,
                                     {first_join_input, second_join_input});

  // Check here if column has been renamed already!
  if (column_renaming_map_.find(first_join_key) != column_renaming_map_.end() ||
      column_renaming_map_.find(second_join_key) !=
          column_renaming_map_.end()) {
    // TODO(Kaspar): Temporary.
    throw std::runtime_error("Join columns have been renamed already!");
  }

  auto base_column = std::to_string(operation_counter_++) + "_column";

  column_renaming_map_.insert({first_join_key, base_column});
  column_renaming_map_.insert({second_join_key, base_column});

  generate_join_offset_ = join_name;
  required_join_offset_ = join_name;

  operations_[join_name].desired_columns.push_back(base_column);
  operations_[join_name].desired_column_locations.insert({base_column, {0}});
  operations_[join_name].sorted_by_column = base_column;

  return join_name;
}

// Just one at a time for these at the moment.
auto SQLQueryCreator::RegisterAddition(std::string input,
                                       std::string column_name,
                                       bool make_negative, double value)
    -> std::string {
  // Check here if column has been renamed already!
  if (column_renaming_map_.find(column_name) != column_renaming_map_.end()) {
    column_name = column_renaming_map_.at(column_name);
  }

  auto addition_name =
      RegisterOperation(QueryOperationType::kAddition, {std::move(input)});
  operations_[addition_name].desired_columns.push_back(column_name);
  operations_[addition_name].operation_columns.push_back(column_name);
  std::vector<int> initial_addition_values = {
      static_cast<const int>(make_negative), static_cast<int>(value * 100)};
  operations_[addition_name].operation_params.push_back(
      initial_addition_values);

  operations_[addition_name].desired_column_locations.insert(
      {column_name, {0, 2, 4, 6, 8, 10, 12, 14}});

  return addition_name;
}
auto SQLQueryCreator::RegisterMultiplication(
    std::string input, const std::string& first_column_name,
    const std::string& second_column_name,
    const std::string& result_column_name) -> std::string {
  // Check here if column has been renamed already!
  if (column_renaming_map_.find(first_column_name) !=
          column_renaming_map_.end() ||
      column_renaming_map_.find(result_column_name) !=
          column_renaming_map_.end()) {
    // TODO(Kaspar): Temporary.
    throw std::runtime_error(
        "Multiplication columns have been renamed already!");
  }

  columns_.insert({result_column_name, {ColumnDataType::kDecimal, 1}});
  auto multiplication_name = RegisterOperation(
      QueryOperationType::kMultiplication, {std::move(input)});

  auto result_column = std::to_string(operation_counter_++) + "_column";

  column_renaming_map_.insert({result_column_name, result_column});
  column_renaming_map_.insert({first_column_name, result_column});

  operations_[multiplication_name].desired_column_locations.insert(
      {result_column, {0, 4, 8, 12}});
  operations_[multiplication_name].paired_to_column.insert(
      {second_column_name, result_column});

  operations_[multiplication_name].desired_columns.push_back(result_column);
  operations_[multiplication_name].desired_columns.push_back(
      second_column_name);

  operations_[multiplication_name].operation_columns.push_back(result_column);
  operations_[multiplication_name].operation_columns.push_back(
      second_column_name);
  return multiplication_name;
}
auto SQLQueryCreator::RegisterAggregation(std::string input,
                                          std::string column_name)
    -> std::string {
  // Check here if column has been renamed already!
  if (column_renaming_map_.find(column_name) != column_renaming_map_.end()) {
    column_name = column_renaming_map_.at(column_name);
  }

  auto new_name = RegisterOperation(QueryOperationType::kAggregationSum,
                                    {std::move(input)});
  operations_[new_name].desired_columns.push_back(column_name);
  operations_[new_name].operation_columns.push_back(column_name);
  return new_name;
}

auto SQLQueryCreator::GetComparisonColumnName(const std::string& filter_id,
                                              const std::string& column_name)
    -> std::string {
  // If column name already has 4 comparisons
  // Check if column name is duplicated
  // Take the new name for the duplicated_column_list
  // Check again until you get a name that is valid

  std::string current_column_name = column_name;
  bool columns_name_found = false;
  while (!columns_name_found) {
    if (const auto& [it, inserted] =
            filter_comparison_column_counts_[filter_id].try_emplace(
                current_column_name, 1);
        !inserted) {
      // 4 is the maximum at the moment
      // TODO: remove hard coded value!
      if (it->second < 4) {
        it->second++;
        columns_name_found = true;
      } else {
        // Need to find a new column name:
        if (column_name == current_column_name) {
          // Current column_name is original
          if (duplicated_columns_.find(column_name) !=
              duplicated_columns_.end()) {
            // Get the first duplicated column name
            current_column_name = duplicated_columns_.at(column_name).front();
          } else {
            current_column_name += "_0";
            if (columns_.find(current_column_name) != columns_.end()) {
              throw std::runtime_error(
                  "Can't duplicate columns due to name clashes");
            } else {
              duplicated_columns_.insert({column_name, {current_column_name}});
              original_of_an_duplicated_column_.insert(
                  {current_column_name, column_name});
            }
          }
        } else {
          // We are already checking a duplicated column name
          // TODO: This shouldn't error but still needs checks for robustness!
          int duplicated_column_index = stoi(current_column_name.substr(
              current_column_name.find_last_of('_') + 1));
          if (duplicated_columns_.at(column_name).size() ==
              duplicated_column_index + 1) {
            current_column_name =
                column_name + "_" + std::to_string(duplicated_column_index + 1);
            duplicated_columns_.at(column_name).push_back(current_column_name);
            original_of_an_duplicated_column_.insert(
                {current_column_name, column_name});
          } else {
            current_column_name = duplicated_columns_.at(column_name)
                                      .at(duplicated_column_index + 1);
          }
        }
      }
    } else {
      columns_name_found = true;
    }
  }

  if (std::find(operations_[filter_id].desired_columns.begin(),
                operations_[filter_id].desired_columns.end(),
                current_column_name) ==
      operations_[filter_id].desired_columns.end()) {
    operations_[filter_id].desired_columns.push_back(current_column_name);
  }

  return current_column_name;
};

auto SQLQueryCreator::AddStringComparison(const std::string& filter_id,
                                          const std::string& column_name,
                                          CompareFunctions comparison_type,
                                          const std::string& compare_value)
    -> int {
  /*std::cout << operation_counter_ << ":";
  std::cout << "String " << filter_id;
  std::cout << " " << column_name;
  std::cout << " " << compare_value;
  std::cout << std::endl;*/
  auto current_column_name = GetComparisonColumnName(filter_id, column_name);

  FilterOperation new_operation;
  new_operation.column_name = current_column_name;
  new_operation.operation = comparison_type;
  new_operation.comparison_values = std::make_pair(
      compare_value,
      static_cast<int>(columns_.at(column_name).second *
                       column_sizes_.at(columns_.at(column_name).first)));

  filter_operations_[filter_id].insert({operation_counter_, new_operation});
  filter_operations_relations_[filter_id][0].push_back(operation_counter_);
  return operation_counter_++;
}
auto SQLQueryCreator::AddDateComparison(const std::string& filter_id,
                                        const std::string& column_name,
                                        CompareFunctions comparison_type,
                                        int year, int month, int day) -> int {
  /*std::cout << operation_counter_ << ":";
  std::cout << "Date: " << filter_id;
  std::cout << " " << column_name;
  std::cout << " " << std::to_string(year);
  std::cout << " " << std::to_string(month);
  std::cout << " " << std::to_string(day);
  std::cout << std::endl;*/
  auto current_column_name = GetComparisonColumnName(filter_id, column_name);

  FilterOperation new_operation;
  new_operation.column_name = current_column_name;
  new_operation.operation = comparison_type;
  // TODO(Kaspar): Need checks that the format is correct!
  std::vector<int> comparison_values = {year * 10000 + month * 100 + day};
  new_operation.comparison_values = comparison_values;

  filter_operations_[filter_id].insert({operation_counter_, new_operation});
  filter_operations_relations_[filter_id][0].push_back(operation_counter_);
  return operation_counter_++;
}
auto SQLQueryCreator::AddIntegerComparison(const std::string& filter_id,
                                           const std::string& column_name,
                                           CompareFunctions comparison_type,
                                           int compare_value) -> int {
  /*std::cout << operation_counter_ << ":";
  std::cout << "Integer: " << filter_id;
  std::cout << " " << column_name;
  std::cout << " " << std::to_string(compare_value);
  std::cout << std::endl;*/
  auto current_column_name = GetComparisonColumnName(filter_id, column_name);

  FilterOperation new_operation;
  new_operation.column_name = current_column_name;
  new_operation.operation = comparison_type;
  std::vector<int> comparison_values = {compare_value};
  new_operation.comparison_values = comparison_values;

  filter_operations_[filter_id].insert({operation_counter_, new_operation});
  filter_operations_relations_[filter_id][0].push_back(operation_counter_);
  return operation_counter_++;
}
auto SQLQueryCreator::AddDoubleComparison(const std::string& filter_id,
                                          const std::string& column_name,
                                          CompareFunctions comparison_type,
                                          double compare_value) -> int {
  /*std::cout << operation_counter_ << ":";
  std::cout << "Double: " << filter_id;
  std::cout << " " << column_name;
  std::cout << " " << std::to_string(compare_value);
  std::cout << std::endl;*/
  auto current_column_name = GetComparisonColumnName(filter_id, column_name);

  FilterOperation new_operation;
  new_operation.column_name = current_column_name;
  new_operation.operation = comparison_type;
  // TODO(Kaspar): Fix this hardcoded stuff!
  std::vector<int> comparison_values = {0,
                                        static_cast<int>(compare_value * 100)};
  new_operation.comparison_values = comparison_values;

  filter_operations_[filter_id].insert({operation_counter_, new_operation});
  filter_operations_relations_[filter_id][0].push_back(operation_counter_);
  return operation_counter_++;
}

auto SQLQueryCreator::AddOr(const std::string& filter_id,
                            const std::vector<int>& comparison_ids) -> int {
  /*std::cout << operation_counter_ << ":";
  std::cout << "OR: " << filter_id;
  for (const auto id : comparison_ids) {
    std::cout << " " << std::to_string(id);
  }
  std::cout << std::endl;*/
  auto or_id = MakeANewClause(filter_id, comparison_ids, false);
  for (auto const& used_id : comparison_ids) {
    RemoveFromClause(filter_id, 0, used_id);
  }
  filter_operations_relations_[filter_id][0].push_back(or_id);
  return or_id;
}

auto SQLQueryCreator::AddAnd(const std::string& filter_id,
                             const std::vector<int>& comparison_ids) -> int {
  /*std::cout << operation_counter_ << ":";
  std::cout << "AND: " << filter_id;
  for (const auto id : comparison_ids) {
    std::cout << " " << std::to_string(id);
  }
  std::cout << std::endl;*/
  auto and_id = MakeANewClause(filter_id, comparison_ids, true);
  for (auto const& used_id : comparison_ids) {
    RemoveFromClause(filter_id, 0, used_id);
  }
  filter_operations_relations_[filter_id][0].push_back(and_id);
  return and_id;
}

auto SQLQueryCreator::AddNot(const std::string& /*filter_id*/,
                             const std::vector<int>& /*comparison_ids*/)
    -> int {
  // For each AND or OR clause when you hold a literal you have to say if it is
  // negative or positive.
  /*std::cout << operation_counter_ << ":";
  std::cout << "NOT: " << filter_id;
  for (const auto id : comparison_ids) {
    std::cout << " " << std::to_string(id);
  }
  std::cout << std::endl;*/
  throw std::runtime_error("Not implemented yet.");
}