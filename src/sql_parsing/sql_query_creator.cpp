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

#include <iostream>

using orkhestrafs::sql_parsing::SQLQueryCreator;

auto SQLQueryCreator::ExportInputDef() -> std::string {
  return "benchmark_Q19_SF001.json";
}

auto SQLQueryCreator::RegisterOperation(QueryOperationType operation_type)
    -> std::string {
  return "";
}

auto SQLQueryCreator::RegisterTable(std::string filename,
                                    std::vector<TableColumn> columns,
                                    int row_count) -> std::string {
  return "";
}

auto SQLQueryCreator::RegisterFilter(std::string input) -> std::string {
  return "";
}

auto SQLQueryCreator::RegisterSort(std::string input) -> std::string {
  return "";
}

auto SQLQueryCreator::RegisterJoin(std::string first_input,
                                   std::string first_join_key,
                                   std::string second_input,
                                   std::string second_join_key) -> std::string {
  return "";
}

auto SQLQueryCreator::RegisterAddition(std::string input) -> std::string {
  return "";
}

auto SQLQueryCreator::RegisterMultiplication(std::string input) -> std::string {
  return "";
}

auto SQLQueryCreator::RegisterAggregation(std::string input) -> std::string {
  return "";
}

auto SQLQueryCreator::AddComparison(std::string filter_id) -> int { return 0; }

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