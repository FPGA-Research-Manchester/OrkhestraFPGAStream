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

#include "sql_parser.hpp"

#include <iostream>
#include "sql_json_reader.hpp"

using orkhestrafs::sql_parsing::SQLParser;
using orkhestrafs::sql_parsing::SQLJSONReader;

void SQLParser::CreatePlan(SQLQueryCreator& sql_creator, std::string query_filename) {
  std::cout<<"Parsing: " << query_filename << std::endl;
  std::map<int, std::vector<std::string>> explain_data;
  // Actually the query needs to get parsed by the Python script first!
  SQLJSONReader::ReadQuery(query_filename, explain_data);
  // After reading the data in you just use the API to write everything in.
  // Have to do it in order from leafs to root node.
}