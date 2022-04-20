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

#include "sql_json_writer.hpp"

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

using orkhestrafs::sql_parsing::SQLJSONWriter;
using rapidjson::Document;
using rapidjson::FileWriteStream;
using rapidjson::Value;
using rapidjson::Writer;

void SQLJSONWriter::WriteQuery(
    std::string filename, std::map<std::string, InputNodeParameters> data) {
  Document document;
  document.SetObject();
  for (const auto& [key, value] : data) {
    Value json_key;
    // TODO: Look into writing!
    json_key.SetString(key.c_str(), key.size(), document.GetAllocator());
    document.AddMember(json_key, value, document.GetAllocator());
  }
  FILE* file_pointer = fopen(filename.c_str(), "wb");  // non-Windows use "w"

  if (!file_pointer) {
    throw std::runtime_error("Couldn't find: " + filename);
  }

  char write_buffer[8192];
  FileWriteStream output_stream(file_pointer, write_buffer,
                                sizeof(write_buffer));

  Writer<FileWriteStream> writer(output_stream);
  document.Accept(writer);

  fclose(file_pointer);
}