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

#include "sql_json_reader.hpp"

#include <iostream>

using orkhestrafs::sql_parsing::SQLJSONReader;

void SQLJSONReader::ReadQuery(
    std::string filename, std::map<int, std::vector<std::string>>& data) {
  std::cout<<"Reading: " << filename << std::endl;
}