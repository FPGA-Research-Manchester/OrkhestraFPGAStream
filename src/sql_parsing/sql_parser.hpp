﻿/*
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
 * @brief Class to Read SQL and create a Query Plan
 */
class SQLParser {
 public:
  static void CreatePlan(SQLQueryCreator& sql_creator,
                         const std::string& query_filename,
                         const std::string& database_name);
  static void PrintResults(const std::string& query_filename,
                           const std::string& database_name);
};
}  // namespace orkhestrafs::sql_parsing