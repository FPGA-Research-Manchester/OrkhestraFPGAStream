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
#include "query_scheduling_helper.hpp"

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
