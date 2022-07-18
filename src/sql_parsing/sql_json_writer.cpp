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

#include <stdexcept>

#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/writer.h"

using orkhestrafs::sql_parsing::SQLJSONWriter;
using rapidjson::Document;
using rapidjson::FileWriteStream;
using rapidjson::PrettyWriter;
using rapidjson::Value;

void SQLJSONWriter::WriteQuery(
    const std::string& filename,
    const std::map<std::string, InputNodeParameters>& data) {
  Document document;
  document.SetObject();
  for (const auto& [node_name, node_value_map] : data) {
    Value node_name_value;
    node_name_value.SetString(node_name.c_str(), node_name.size(),
                              document.GetAllocator());
    Value node_params_value(rapidjson::kObjectType);
    for (const auto& [param_field_name, param_field_value] : node_value_map) {
      Value param_name_value;
      param_name_value.SetString(param_field_name.c_str(),
                                 param_field_name.size(),
                                 document.GetAllocator());
      if (std::holds_alternative<std::string>(param_field_value)) {
        Value param_value;
        param_value.SetString(std::get<std::string>(param_field_value).c_str(),
                              std::get<std::string>(param_field_value).size(),
                              document.GetAllocator());
        node_params_value.AddMember(param_name_value, param_value,
                                    document.GetAllocator());
      } else if (std::holds_alternative<std::vector<std::string>>(
                     param_field_value)) {
        Value param_array(rapidjson::kArrayType);
        auto data_array = std::get<std::vector<std::string>>(param_field_value);
        for (const auto& array_value : data_array) {
          if (array_value.empty()) {
            Value param_value;  // Write NULL
            param_array.PushBack(param_value, document.GetAllocator());
          } else {
            Value param_value;
            param_value.SetString(array_value.c_str(), array_value.size(),
                                  document.GetAllocator());
            param_array.PushBack(param_value, document.GetAllocator());
          }
        }
        node_params_value.AddMember(param_name_value, param_array,
                                    document.GetAllocator());
      } else if (std::holds_alternative<std::map<std::string, OperationParams>>(
                     param_field_value)) {
        Value param_object(rapidjson::kObjectType);
        for (const auto& [stream_param_field_name, stream_param_field_value] :
             std::get<std::map<std::string, OperationParams>>(
                 param_field_value)) {
          Value stream_param_name_value;
          stream_param_name_value.SetString(stream_param_field_name.c_str(),
                                            stream_param_field_name.size(),
                                            document.GetAllocator());
          Value stream_param_array(rapidjson::kArrayType);
          for (const auto& array_value : stream_param_field_value) {
            Value current_stream_param_array(rapidjson::kArrayType);
            if (std::holds_alternative<std::vector<int>>(array_value)) {
              for (const auto& int_value :
                   std::get<std::vector<int>>(array_value)) {
                Value param_value(int_value);
                current_stream_param_array.PushBack(param_value,
                                                    document.GetAllocator());
              }
            } else {
              auto [string_value, int_value] =
                  std::get<std::pair<std::string, int>>(array_value);
              Value param_value;
              param_value.SetString(string_value.c_str(), string_value.size(),
                                    document.GetAllocator());
              current_stream_param_array.PushBack(param_value,
                                                  document.GetAllocator());
              Value int_param_value(int_value);
              current_stream_param_array.PushBack(int_param_value,
                                                  document.GetAllocator());
            }
            stream_param_array.PushBack(current_stream_param_array,
                                        document.GetAllocator());
          }
          param_object.AddMember(stream_param_name_value, stream_param_array,
                                 document.GetAllocator());
        }
        node_params_value.AddMember(param_name_value, param_object,
                                    document.GetAllocator());
      } else {
        throw std::runtime_error("Wrong data given to write!");
      }
    }
    document.AddMember(node_name_value, node_params_value,
                       document.GetAllocator());
  }
  FILE* file_pointer = fopen(filename.c_str(), "wb");  // non-Windows use "w"

  if (!file_pointer) {
    throw std::runtime_error("Couldn't find: " + filename);
  }

  char write_buffer[8192];
  FileWriteStream output_stream(file_pointer, write_buffer,
                                sizeof(write_buffer));

  PrettyWriter<FileWriteStream> writer(output_stream);
  writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);
  document.Accept(writer);

  fclose(file_pointer);
}