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
#include <memory>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

using orkhestrafs::sql_parsing::SQLJSONReader;

using rapidjson::Document;
using rapidjson::FileReadStream;

void SQLJSONReader::ReadQuery(const std::string& filename,
                              std::map<int, std::vector<std::string>>& data) {
  // TODO(Kaspar): Same as rapidjosn_reader::Read - Remove the duplication
  FILE* file_pointer = fopen(filename.c_str(), "r");

  if (!file_pointer) {
    throw std::runtime_error("Couldn't find: " + filename);
  }

  char read_buffer[8192];
  FileReadStream input_stream(file_pointer, read_buffer, sizeof(read_buffer));

  auto document = std::make_unique<Document>();
  document->ParseStream(input_stream);

  fclose(file_pointer);

  const std::string params = "params";
  const std::string type = "type";

  for (const auto& element : document->GetObject()) {
    int current_key = std::stoi(element.name.GetString());
    std::vector<std::string> current_params;
    current_params.emplace_back(
        element.value.GetObject()[type.c_str()].GetString());
    for (const auto& param :
         element.value.GetObject()[params.c_str()].GetArray()) {
      if (param.IsInt()) {
        current_params.push_back(std::to_string(param.GetInt()));
      } else if (param.IsString()) {
        current_params.emplace_back(param.GetString());
      } else {
        throw std::runtime_error("Incorrect type");
      }
    }
    data.insert({current_key, current_params});
  }
}