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

#include "sql_query_creator.hpp"

namespace orkhestrafs::sql_parsing {

/**
 * @brief Class to programmatically create queries.
 */
class Q19Creator {
 private:
  static auto CreateLineitemTable(SQLQueryCreator* sql_creator, int row_count,
                                  std::string filename) -> std::string;
  static auto CreatePartTable(SQLQueryCreator* sql_creator, int row_count,
                              std::string filename) -> std::string;
  static void ConfigureQ19FirstFilter(SQLQueryCreator& sql_creator,
                                      std::string& first_filter);
  static void ConfigureQ19SecondFilter(SQLQueryCreator& sql_creator,
                                       std::string& second_filter);

 public:
  static void CreateQ19TPCH(SQLQueryCreator& sql_creator);
};
}  // namespace orkhestrafs::sql_parsing