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

#include "rapidjson_reader.hpp"

#include <cstdio>
#include <stdexcept>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

using easydspi::dbmstodspi::RapidJSONReader;
using rapidjson::Document;
using rapidjson::FileReadStream;

auto RapidJSONReader::Read(const std::string& json_filename)
    -> std::unique_ptr<Document> {
  FILE* file_pointer = fopen(json_filename.c_str(), "r");

  if (!file_pointer) {
    throw std::runtime_error("Couldn't find: " + json_filename);
  }

  char read_buffer[8192];
  FileReadStream input_stream(file_pointer, read_buffer, sizeof(read_buffer));

  auto document = std::make_unique<Document>();
  document->ParseStream(input_stream);

  fclose(file_pointer);
  return document;
}

auto RapidJSONReader::ReadInputDefinition(std::string json_filename)
    -> std::map<std::string, RapidJSONReader::InputNodeParameters> {
  const auto document = Read(json_filename);
  std::map<std::string, RapidJSONReader::InputNodeParameters> input_node_map;
  for (const auto& node : document->GetObject()) {
    InputNodeParameters node_parameters_map;
    for (const auto& node_parameter : node.value.GetObject()) {
      if (!node_parameter.value.IsNull()) {
        if (node_parameter.value.IsString()) {
          GetOperationType(node_parameters_map, node_parameter);
        } else if (node_parameter.value.IsArray()) {
          GetIOStreamFilesAndDependencies(node_parameter, node_parameters_map);
        } else if (node_parameter.value.IsObject()) {
          GetOperationParameters(node_parameter, node_parameters_map);
        } else {
          throw std::runtime_error(
              "Something went wrong. Please use a verifier!");
        }
      }
    }
    input_node_map.insert({node.name.GetString(), node_parameters_map});
  }
  return input_node_map;
}

void RapidJSONReader::GetOperationParameters(
    const rapidjson::GenericMember<
        rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& node_parameter,
    JSONReaderInterface::InputNodeParameters& node_parameters_map) {
  std::map<std::string, std::vector<std::vector<int>>> operation_parameters_map;
  for (const auto& operation_parameter : node_parameter.value.GetObject()) {
    std::vector<std::vector<int>> parameter_vectors;
    for (const auto& given_parameter_vectors :
         operation_parameter.value.GetArray()) {
      std::vector<int> parameters;
      if (given_parameter_vectors.Size() != 0 &&
          given_parameter_vectors[0].IsString()) {
        parameters =
            ConvertCharStringToAscii(given_parameter_vectors[0].GetString(),
                                     given_parameter_vectors[1].GetInt());
      } else {
        for (const auto& parameter : given_parameter_vectors.GetArray()) {
          parameters.push_back(parameter.GetInt());
        }
      }
      parameter_vectors.push_back(parameters);
    }
    operation_parameters_map.insert(
        {operation_parameter.name.GetString(), parameter_vectors});
  }
  node_parameters_map.insert(
      {node_parameter.name.GetString(), operation_parameters_map});
}

void RapidJSONReader::GetIOStreamFilesAndDependencies(
    const rapidjson::GenericMember<
        rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& node_parameter,
    JSONReaderInterface::InputNodeParameters& node_parameters_map) {
  std::vector<std::string> string_array;
  for (const auto& value_array_string : node_parameter.value.GetArray()) {
    if (value_array_string.IsString()) {
      string_array.emplace_back(value_array_string.GetString());
    } else {
      string_array.emplace_back("");
    }
  }
  node_parameters_map.insert({node_parameter.name.GetString(), string_array});
}

void RapidJSONReader::GetOperationType(
    JSONReaderInterface::InputNodeParameters& node_parameters_map,
    const rapidjson::GenericMember<
        rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>& node_parameter) {
  node_parameters_map.insert(
      {node_parameter.name.GetString(), node_parameter.value.GetString()});
}

auto RapidJSONReader::ReadDataSizes(std::string json_filename)
    -> std::map<std::string, double> {
  const auto document = Read(json_filename);
  std::map<std::string, double> value_map;
  for (const auto& element : document->GetObject()) {
    value_map.insert({element.name.GetString(), element.value.GetDouble()});
  }
  return value_map;
}

auto RapidJSONReader::ReadReqMemorySpace(std::string json_filename)
    -> std::map<std::string, int> {
  const auto document = Read(json_filename);
  std::map<std::string, int> value_map;
  for (const auto& element : document->GetObject()) {
    value_map.insert({element.name.GetString(), element.value.GetInt()});
  }
  return value_map;
}

auto RapidJSONReader::ReadAcceleratorLibrary(std::string json_filename)
    -> std::map<std::vector<std::pair<std::string, std::vector<int>>>,
                std::string> {
  std::string module_combination_names_field = "module_combinations";
  std::string accelerator_name_field = "accelerators";
  std::string module_name_field = "name";
  std::string module_capacity_field = "capacity";
  const auto document = Read(json_filename);
  auto* document_ptr = document.get();
  auto module_combination_names =
      (*document_ptr)[module_combination_names_field.c_str()].GetArray();

  std::map<std::vector<std::pair<std::string, std::vector<int>>>, std::string>
      value_map;
  for (const auto& combination_name : module_combination_names) {
    std::vector<std::pair<std::string, std::vector<int>>> key;
    for (const auto& accelerator_module :
         (*document_ptr)[combination_name.GetString()].GetArray()) {
      const auto combination_object = accelerator_module.GetObject();
      std::string module_name =
          combination_object[module_name_field.c_str()].GetString();
      std::vector<int> module_capacity;
      for (const auto& capacity_value :
           combination_object[module_capacity_field.c_str()].GetArray()) {
        module_capacity.push_back(capacity_value.GetInt());
      }
      key.emplace_back(module_name, module_capacity);
    }
    value_map.insert({key, (*document_ptr)[accelerator_name_field.c_str()]
                               .GetObject()[combination_name.GetString()]
                               .GetString()});
  }
  return value_map;
}

auto RapidJSONReader::ConvertCharStringToAscii(const std::string& input_string,
                                               int output_size)
    -> std::vector<int> {
  if (input_string.length() > output_size * 4) {
    throw std::runtime_error(
        (input_string + " is longer than " + std::to_string(output_size * 4))
            .c_str());
  }
  std::vector<int> integer_values(output_size, 0);
  for (int i = 0; i < input_string.length(); i++) {
    integer_values[i / 4] += int(input_string[i]) << (3 - (i % 4)) * 8;
  }
  return integer_values;
}