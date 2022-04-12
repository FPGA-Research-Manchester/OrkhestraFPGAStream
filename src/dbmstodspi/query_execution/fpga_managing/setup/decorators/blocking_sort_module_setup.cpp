/*
Copyright 2021 University of Manchester

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

#include "blocking_sort_module_setup.hpp"

#include <algorithm>
#include <stdexcept>

#include "query_scheduling_helper.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::table_data::SortedSequence;
using orkhestrafs::dbmstodspi::BlockingSortModuleSetup;
using orkhestrafs::dbmstodspi::QuerySchedulingHelper;

auto BlockingSortModuleSetup::GetWorstCaseProcessedTables(
    const std::vector<int>& min_capacity,
    const std::vector<std::string>& input_tables,
    const std::map<std::string, TableMetadata>& data_tables)
    -> std::map<std::string, TableMetadata> {
  std::map<std::string, TableMetadata> resuling_tables;
  for (const auto& table_name : input_tables) {
    if (QuerySchedulingHelper::IsTableSorted(data_tables.at(table_name))) {
      resuling_tables.insert({table_name, data_tables.at(table_name)});
    } else {
      auto new_table_name = table_name + "_fully_sorted";
      auto new_table_data = data_tables.at(table_name);
      new_table_data.sorted_status = {{0, new_table_data.record_count}};
      resuling_tables.insert({new_table_name, new_table_data});
    }
  }
  return resuling_tables;
}

auto BlockingSortModuleSetup::UpdateDataTable(
    const std::vector<int>& module_capacity,
    const std::vector<std::string>& input_table_names,
    std::map<std::string, TableMetadata>& resulting_tables) -> bool {
  if (input_table_names.size() != 1) {
    throw std::runtime_error("Wrong number of tables!");
  }
  if (module_capacity.size() != 1) {
    throw std::runtime_error("Wrong merge sort capacity given!");
  }
  if (QuerySchedulingHelper::IsTableSorted(
          resulting_tables.at(input_table_names.front()))) {
    throw std::runtime_error("Table is sorted already!");
  }

  const auto& table_name = input_table_names.front();
  const auto& current_table_sorted_status =
      resulting_tables.at(table_name).sorted_status;
  const auto& current_table_record_count =
      resulting_tables.at(table_name).record_count;

  std::vector<SortedSequence> new_sorted_sequences;
  new_sorted_sequences.reserve(current_table_sorted_status.size());
  if (current_table_sorted_status.empty()) {
    new_sorted_sequences.emplace_back(
        0, std::min(module_capacity.front(), current_table_record_count));
  } else {
    int new_sequence_length = 0;
    for (int sequence_index = 0;
         sequence_index <
         std::min(module_capacity.front(),
                  static_cast<int>(current_table_sorted_status.size()));
         sequence_index++) {
      new_sequence_length +=
          current_table_sorted_status.at(sequence_index).length;
    }
    if (module_capacity.front() > current_table_sorted_status.size() &&
        new_sequence_length < current_table_record_count) {
      new_sequence_length +=
          module_capacity.front() - current_table_sorted_status.size();
      new_sequence_length =
          std::min(new_sequence_length, current_table_record_count);
    }
    new_sorted_sequences.emplace_back(0, new_sequence_length);
    if (new_sequence_length < current_table_record_count &&
        module_capacity.front() < current_table_sorted_status.size()) {
      new_sorted_sequences.insert(
          new_sorted_sequences.end(),
          current_table_sorted_status.begin() + module_capacity.front(),
          current_table_sorted_status.end());
      /*for (int sequence_index = module_capacity.front();
           sequence_index < current_sequences.size(); sequence_index++) {
        new_sorted_sequences.push_back(current_sequences.at(sequence_index));
      }*/
    }
  }
  // Fix later
  // current_table.sorted_status = {{0, current_table.record_count}};
  resulting_tables.at(table_name).sorted_status =
      std::move(new_sorted_sequences);
  return QuerySchedulingHelper::IsTableSorted(resulting_tables.at(table_name));
}
