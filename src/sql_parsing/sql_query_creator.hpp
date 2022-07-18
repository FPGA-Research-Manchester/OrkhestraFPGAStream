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
#include <unordered_set>
#include <vector>

#include "operation_types.hpp"
#include "sql_query_data.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::operation_types::QueryOperationType;
using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::sql_parsing::query_data::CompareFunctions;
using orkhestrafs::sql_parsing::query_data::FilterOperation;
using orkhestrafs::sql_parsing::query_data::OperationData;
using orkhestrafs::sql_parsing::query_data::OperationParams;
using orkhestrafs::sql_parsing::query_data::TableColumn;

namespace orkhestrafs::sql_parsing {

using InputNodeParameters =
    std::map<std::string, std::variant<std::string, std::vector<std::string>,
                                       std::map<std::string, OperationParams>>>;

/**
 * @brief Class to programmatically create queries.
 */
class SQLQueryCreator {
 private:
  const std::string input_files_string_ = "input";
  const std::string output_files_string_ = "output";
  const std::string input_nodes_string_ = "previous_nodes";
  const std::string output_nodes_string_ = "next_nodes";
  const std::string operation_string_ = "operation";
  const std::string operation_parameters_string_ = "operation_parameters";
  const std::string input_parameters_string_ = "input_stream_params";
  const std::string output_parameters_string_ = "output_stream_params";
  const std::string operation_specific_params_string_ = "operation_params";

  const int io_param_vector_count_ = 4;
  const int crossbar_offset_ = 0;
  const int data_types_offset_ = 1;
  const int data_sizes_offset_ = 2;
  const int chunk_count_offset_ = 3;

  const std::unordered_map<QueryOperationType, std::string> operation_names_ = {
      {QueryOperationType::kFilter, "Filter"},
      {QueryOperationType::kJoin, "Join"},
      {QueryOperationType::kMergeSort, "MergeSort"},
      {QueryOperationType::kLinearSort, "LinSort"},
      {QueryOperationType::kAddition, "Addition"},
      {QueryOperationType::kMultiplication, "Mul"},
      {QueryOperationType::kAggregationSum, "Sum"}};
  const std::unordered_map<QueryOperationType, std::string>
      operation_enum_strings_ = {
          {QueryOperationType::kFilter, "kFilter"},
          {QueryOperationType::kJoin, "kJoin"},
          {QueryOperationType::kMergeSort, "kMergeSort"},
          {QueryOperationType::kLinearSort, "kLinearSort"},
          {QueryOperationType::kAddition, "kAddition"},
          {QueryOperationType::kMultiplication, "kMultiplication"},
          {QueryOperationType::kAggregationSum, "kAggregationSum"}};
  // TODO(Kaspar): Has to be made global static variable!
  const std::unordered_map<ColumnDataType, double> column_sizes_ = {
      {ColumnDataType::kVarchar, 0.25},
      {ColumnDataType::kDate, 1},
      {ColumnDataType::kDecimal, 2},
      {ColumnDataType::kInteger, 1},
      {ColumnDataType::kNull, 1}};
  // TODO(Kaspar): Has to be made global static variable - And be linked to the
  // module_config_values.hpp!
  const std::unordered_map<CompareFunctions, int> compare_function_mapping_{
      {CompareFunctions::kLessThan, 0},
      {CompareFunctions::kLessThanOrEqual, 1},
      {CompareFunctions::kEqual, 2},
      {CompareFunctions::kGreaterThanOrEqual, 3},
      {CompareFunctions::kGreaterThan, 4},
      {CompareFunctions::kNotEqual, 5}};

  std::unordered_map<std::string, std::unordered_map<int, std::vector<int>>>
      filter_operations_relations_;
  std::unordered_map<std::string, std::unordered_map<int, bool>> is_and_;
  std::unordered_map<std::string, std::unordered_map<int, FilterOperation>>
      filter_operations_;
  std::unordered_map<std::string, bool> is_table_;
  int operation_counter_ = 0;
  std::unordered_set<std::string> input_operations_;
  std::unordered_set<std::string> output_operations_;
  std::unordered_map<std::string, std::pair<ColumnDataType, int>> columns_;
  std::unordered_map<std::string, OperationData> operations_;
  std::unordered_map<std::string, std::vector<std::string>> tables_;
  std::unordered_map<std::string, std::string> column_renaming_map_;

  auto FlattenClauses(const std::string& current_process, int child_term_id,
                      int current_term_id, int new_current_term_id) -> int;
  // Replace ORs in ANDs with A new big OR with ANDs to replace the big AND.
  //
  auto DistributeOrs(const std::string& current_process, int child_term_id,
                     int current_term_id, int new_current_term_id) -> int;
  auto TransformToDnf(const std::string& current_process, int current_term_id)
      -> int;
  auto IsLiteral(const std::string& current_process, int term_id) -> bool;

  void SetAdditionStreamParams(
      const std::string& current_process,
      std::map<std::string, OperationParams>& current_operation_params);
  void SetAggregationStreamParams(
      const std::string& current_process,
      std::map<std::string, OperationParams>& current_operation_params);
  void SetMultiplicationStreamParams(
      const std::string& current_process,
      std::map<std::string, OperationParams>& current_operation_params);
  void SetFilterStreamParams(
      const std::string& current_process,
      std::map<std::string, OperationParams>& current_operation_params);

  auto RegisterOperation(QueryOperationType operation_type,
                         std::vector<std::string> inputs) -> std::string;
  void UpdateRequiredColumns();
  void AddTablesToProcessedOperations(
      std::unordered_set<std::string>& processed_operations);
  void UpdateNextOperationsListIfAvailable(
      std::unordered_set<std::string>& processed_operations,
      std::unordered_set<std::string>& operations_to_process,
      const std::basic_string<char>& process_name);
  void SetInputsForDataMap(const std::string& current_process,
                           InputNodeParameters& current_parameters);
  void SetOutputsForDataMap(
      const std::string& current_process,
      InputNodeParameters& current_parameters,
      std::unordered_set<std::string>& processed_operations,
      std::unordered_set<std::string>& operations_to_process);
  auto ProcessTableColumns(const std::string& current_process, int parent_index,
                           const std::string& table_name) -> int;
  auto CompressNullColumns(const std::string& current_process, int stream_index,
                           int used_column_count) -> int;
  void CopyOutputParamsOfParent(const std::string& current_process,
                                int parent_index, const std::string& parent);
  void CombineOutputStreamParams(const std::string& current_process);
  auto SetIOStreamParams(const std::string& current_process) -> int;
  void SetStreamParamsForDataMap(const std::string& current_process,
                                 InputNodeParameters& current_parameters);
  void FillDataMap(std::unordered_set<std::string> processed_operations,
                   std::unordered_set<std::string> operations_to_process,
                   std::map<std::string, InputNodeParameters>& data_to_write);
  void SetOperationSpecificStreamParamsForDataMap(
      const std::string& current_process,
      std::map<std::string, OperationParams>& current_operation_params);
  void AddColumnFilteringParams(
      const std::string& current_process, int location,
      const std::vector<CompareFunctions>& operations,
      const std::vector<std::variant<std::pair<std::string, int>, int>>&
          comparison_values,
      const std::vector<std::vector<int>>& clause_types,
      const std::vector<std::vector<int>>& clauses,
      OperationParams& resulting_params);
  auto MakeACopyClause(const std::string& current_process, int original_term_id)
      -> int;
  auto MakeANewClause(const std::string& current_process,
                      const std::vector<int>& initial_child_clauses,
                      bool is_and) -> int;
  void RemoveFromClause(const std::string& current_process, int parent_term_id,
                        int removable_term_id);
  void RenameAllColumns();
  auto PlaceColumnsToDesiredPositions(const std::string& current_process,
                                      int stream_index,
                                      int last_needed_column_index,
                                      std::string table_name, int record_size)
      -> int;
  auto MapColumnPositions(
      const std::string& current_process, int stream_index,
      int last_needed_column_index, const std::string& table_name,
      std::map<std::string, std::vector<int>>& column_positions) -> int;
  void PlaceColumnsThatSpanOverMultipleChunks(
      std::vector<std::vector<int>>& crossbar_configuration,
      std::vector<std::string>& chosen_columns,
      const std::map<std::string, std::vector<int>>& column_positions,
      const std::string& current_process);
  void GetCurrentAvailableDesiredPositions(
      const std::map<std::string, std::vector<int>>& column_positions,
      const std::string& current_process,
      std::map<int, std::vector<std::string>>&
          current_available_desired_columns,
      std::map<std::string, std::vector<int>>& left_over_availability);
  static void RemoveUnavailablePositions(
      std::map<int, std::vector<std::string>>&
          current_available_desired_columns,
      std::map<std::string, std::vector<int>>& left_over_availability,
      const std::vector<std::string>& chosen_columns);
  void RemoveAvailabliltyDueToJoinRequirements(
      std::map<int, std::vector<std::string>>&
          current_available_desired_columns,
      std::map<std::string, std::vector<int>>& left_over_availability,
      const std::string& current_process,
      std::vector<std::vector<int>>& crossbar_configuration,
      std::vector<std::string>& chosen_columns,
      const std::map<std::string, std::vector<int>>& column_positions,
      const std::map<std::string, std::string>& pairing_map);
  void SetColumnPlace(
      std::map<int, std::vector<std::string>>&
          current_available_desired_columns,
      std::map<std::string, std::vector<int>>& left_over_availability,
      std::vector<std::vector<int>>& crossbar_configuration,
      std::vector<std::string>& chosen_columns,
      const std::map<std::string, std::vector<int>>& column_positions,
      const std::string& chosen_column_name, int chosen_location,
      const std::map<std::string, std::string>& pairing_map,
      const std::string& current_process);
  static auto UsesMultipleChunks(std::vector<int> position_vector) -> bool;
  void PlaceColumnsToPositionsWithOneAvailableLocation(
      std::map<int, std::vector<std::string>>&
          current_available_desired_columns,
      std::map<std::string, std::vector<int>>& left_over_availability,
      std::vector<std::vector<int>>& crossbar_configuration,
      std::vector<std::string>& chosen_columns,
      const std::map<std::string, std::vector<int>>& column_positions,
      const std::map<std::string, std::string>& pairing_map,
      const std::string& current_process);
  void PlaceGivenColumnsToGivenDesiredLocations(
      std::map<int, std::vector<std::string>>&
          current_available_desired_columns,
      std::map<std::string, std::vector<int>>& left_over_availability,
      std::vector<std::vector<int>>& crossbar_configuration,
      std::vector<std::string>& chosen_columns,
      const std::map<std::string, std::vector<int>>& column_positions,
      const std::map<std::string, std::string>& pairing_map,
      const std::string& current_process);
  void SetJoinOffsetParam(const std::string& current_process);
  void CleanAvailablePositionsAndPlaceColumns(
      std::map<int, std::vector<std::string>>&
          current_available_desired_columns,
      std::map<std::string, std::vector<int>>& left_over_availability,
      const std::string& current_process,
      std::vector<std::vector<int>>& crossbar_configuration,
      std::vector<std::string>& chosen_columns,
      const std::map<std::string, std::vector<int>>& column_positions,
      int stream_index, const std::map<std::string, std::string>& pairing_map);
  void SetJoinStreamParams(
      const std::string& current_process,
      std::map<std::string, OperationParams>& current_operation_params);

 public:
  auto ExportInputDef() -> std::string;
  auto RegisterTable(std::string filename, std::vector<TableColumn> columns,
                     int row_count) -> std::string;
  auto RegisterFilter(std::string input) -> std::string;
  auto RegisterSort(std::string input, std::string column_name) -> std::string;
  auto RegisterJoin(std::string first_input, const std::string& first_join_key,
                    std::string second_input,
                    const std::string& second_join_key) -> std::string;
  auto RegisterAddition(std::string input, std::string column_name,
                        bool make_negative, double value) -> std::string;
  auto RegisterMultiplication(std::string input,
                              const std::string& first_column_name,
                              const std::string& second_column_name,
                              const std::string& result_column_name)
      -> std::string;
  auto RegisterAggregation(std::string input, std::string column_name)
      -> std::string;
  auto AddStringComparison(const std::string& filter_id,
                           const std::string& column_name,
                           CompareFunctions comparison_type,
                           const std::string& compare_value) -> int;
  auto AddDateComparison(const std::string& filter_id,
                         const std::string& column_name,
                         CompareFunctions comparison_type, int year, int month,
                         int day) -> int;
  auto AddIntegerComparison(const std::string& filter_id,
                            const std::string& column_name,
                            CompareFunctions comparison_type, int compare_value)
      -> int;
  auto AddDoubleComparison(const std::string& filter_id,
                           const std::string& column_name,
                           CompareFunctions comparison_type,
                           double compare_value) -> int;
  auto AddOr(const std::string& filter_id,
             const std::vector<int>& comparison_ids) -> int;
  auto AddAnd(const std::string& filter_id,
              const std::vector<int>& comparison_ids) -> int;
  static auto AddNot(const std::string& filter_id,
                     const std::vector<int>& comparison_ids) -> int;
};
}  // namespace orkhestrafs::sql_parsing