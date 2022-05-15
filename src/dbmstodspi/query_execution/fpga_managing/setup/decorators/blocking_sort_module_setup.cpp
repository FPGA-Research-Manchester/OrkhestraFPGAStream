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

  auto& sorted_status =
      resulting_tables.at(input_table_names.front()).sorted_status;
  const auto& module_capacity_value = module_capacity.front();
  if (module_capacity_value > sorted_status.size()) {
    sorted_status.clear();
    sorted_status.push_back(0);
  } else {
    sorted_status.erase(sorted_status.begin() + 1,
                        sorted_status.begin() + module_capacity_value);
  }

  // TODO: Remove this old code 
  //const auto& table_name = input_table_names.front();
  //std::vector<int> new_sorted_sequences;

  //new_sorted_sequences.reserve(
  //    resulting_tables.at(table_name).sorted_status.size());
  //if (resulting_tables.at(table_name).sorted_status.empty()) {
  //    // TODO: Fix this.
  //  throw std::runtime_error("Merge sorter can't sort from 0 currently");
  //} else {
  //  SortDataTableWhileMinimizingMinorRuns(
  //      resulting_tables.at(table_name).sorted_status, new_sorted_sequences,
  //      resulting_tables.at(table_name).record_count, module_capacity.front());
  //  //  SortDataTableWhileMinimizingMajorRuns(
  //  //      resulting_tables.at(table_name).sorted_status, new_sorted_sequences,
  //  //      resulting_tables.at(table_name).record_count,
  //  //      module_capacity.front());
  //}

  //resulting_tables.at(table_name).sorted_status =
  //    std::move(new_sorted_sequences);
  return QuerySchedulingHelper::IsTableSorted(
      resulting_tables.at(input_table_names.front()));
}

void BlockingSortModuleSetup::SortDataTableWhileMinimizingMinorRuns(
    const std::vector<int>& old_sorted_sequences,
    std::vector<int>& new_sorted_sequences, int record_count,
    int module_capacity) {
  // Not supported currently. Assume that all elements belong to a sequence!
  /*int new_sequence_length = 0;
  for (int sequence_index = 0;
       sequence_index <
       std::min(module_capacity, static_cast<int>(old_sorted_sequences.size()));
       sequence_index++) {
    new_sequence_length += old_sorted_sequences.at(sequence_index).length;
  }
  if (module_capacity > old_sorted_sequences.size() &&
      new_sequence_length < record_count) {
    new_sequence_length += module_capacity - old_sorted_sequences.size();
    new_sequence_length = std::min(new_sequence_length, record_count);
  }*/
  new_sorted_sequences.emplace_back(0);
  if (module_capacity < old_sorted_sequences.size()) {
    new_sorted_sequences.insert(new_sorted_sequences.end(),
                                old_sorted_sequences.begin() + module_capacity,
                                old_sorted_sequences.end());
  }
}
void BlockingSortModuleSetup::SortDataTableWhileMinimizingMajorRuns(
    const std::vector<int>& old_sorted_sequences,
    std::vector<int>& new_sorted_sequences, int record_count,
    int module_capacity) {
  // Assuming the sequences are not empty and they're in correct order.
  // TODO: Remove this error
    // Not supported currently!
  /* int old_sequence_length = 0;
  for (const auto& sequence : old_sorted_sequences) {
    old_sequence_length += sequence.length;
  }
  if (old_sequence_length < record_count) {
    throw std::runtime_error(
        "Sorting single elements is not supported currently!");
  }
  // If there are less sequences than that then just return one big sequence
  if (old_sorted_sequences.size() < module_capacity){
    new_sorted_sequences.emplace_back(0, old_sequence_length);
  } else {
    // Find the smallest set of sequences to merge - Not entirely optimal
    int window_count = old_sorted_sequences.size() + 1 - module_capacity;
    std::vector<int> window_lengths (window_count, 0);
    // TODO: Make this one more clever.
    for (int window_location = 0; window_location<window_count; window_location++){
      for (int sequence_index = window_location; sequence_index<window_location+record_count; sequence_index++){
        window_lengths[window_location]+=old_sorted_sequences[sequence_index].length;
      }
    }
    int min_window_index = std::distance(window_lengths.begin(), std::min_element(window_lengths.begin(), window_lengths.end()));
    // Add sequences before window
    int start_point = 0;
    for (int sequence_index = 0; sequence_index < min_window_index; sequence_index++){
      start_point+= old_sorted_sequences[sequence_index].length;
      new_sorted_sequences.push_back(old_sorted_sequences[sequence_index]);
    }
    // Add window
    new_sorted_sequences.emplace_back(start_point, window_lengths.at(min_window_index));
    // If there are sequences to add after the window - add them.
    if (start_point + window_lengths.at(min_window_index) < record_count &&
        module_capacity < old_sorted_sequences.size()) {
      new_sorted_sequences.insert(new_sorted_sequences.end(),
                                  old_sorted_sequences.begin() + min_window_index + module_capacity,
                                  old_sorted_sequences.end());
    }
  }*/
}
