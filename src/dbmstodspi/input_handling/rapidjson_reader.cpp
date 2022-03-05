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

using orkhestrafs::dbmstodspi::RapidJSONReader;
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

auto RapidJSONReader::ReadAllTablesData(std::string json_filename)
    -> std::vector<TableMetaDataStringMap> {
  std::string filename_field = "filename";
  std::string record_size_field = "record_size";
  std::string record_count_field = "record_count";
  std::string sorted_status_field = "sorted_status";
  const auto document = Read(json_filename);
  auto* document_ptr = document.get();
  auto tables_parameters_vector = (*document_ptr).GetArray();

  std::vector<TableMetaDataStringMap> table_data_vector;
  for (const auto& table_parameters_ptr : tables_parameters_vector) {
    TableMetaDataStringMap current_table_value_map;
    auto table_parameters = table_parameters_ptr.GetObject();
    current_table_value_map.insert(
        {filename_field, table_parameters[filename_field.c_str()].GetString()});
    current_table_value_map.insert(
        {record_size_field,
         table_parameters[record_size_field.c_str()].GetInt()});
    current_table_value_map.insert(
        {record_count_field,
         table_parameters[record_count_field.c_str()].GetInt()});
    std::vector<std::vector<int>> sorted_sequences;
    for (const auto& sorted_sequence :
         table_parameters[sorted_status_field.c_str()].GetArray()) {
      std::vector<int> sequence_params;
      for (const auto& sequence_param : sorted_sequence.GetArray()) {
        sequence_params.push_back(sequence_param.GetInt());
      }
      sorted_sequences.push_back(sequence_params);
    }
    current_table_value_map.insert({sorted_status_field, sorted_sequences});
    table_data_vector.push_back(current_table_value_map);
  }
  return table_data_vector;
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

auto RapidJSONReader::ReadValueMap(std::string json_filename)
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

auto RapidJSONReader::ReadHWLibraryData(std::string json_filename)
    -> HWLibraryStringMap {
  std::string bitstreams_field = "bitstreams";
  std::string start_locations_field = "start_locations";
  std::string locations_field = "locations";
  std::string length_field = "length";
  std::string capacity_field = "capacity";
  std::string resource_string_field = "string";
  std::string is_backwards_field = "is_backwards";

  const auto document = Read(json_filename);
  auto* document_ptr = document.get();
  auto operator_functions_list = (*document_ptr).GetObject();

  //  using HWLibraryStringMap = std::map<
  //      std::string,
  //      std::pair<std::map<std::string,
  //                         std::map<std::string,
  //                            std::variant<std::vector<int>,
  //                            int,
  //                            std::string>>>,
  //                std::vector<std::vector<std::string>>>>;

  HWLibraryStringMap resulting_string_map;
  for (const auto& operator_parameters_object : operator_functions_list) {
    std::string operation_type = operator_parameters_object.name.GetString();
    auto operator_parameters = operator_parameters_object.value.GetObject();

    std::map<
        std::string,
        std::map<std::string, std::variant<std::vector<int>, int, std::string>>>
        operation_module_bitstream_map;
    for (const auto& bitstream_object :
         operator_parameters[bitstreams_field.c_str()].GetObject()) {
      std::map<std::string, std::variant<std::vector<int>, int, std::string>>
          bitstream_parameters_map;
      std::string bitstream_name = bitstream_object.name.GetString();
      std::vector<int> bitstream_locations;
      for (const auto& location_integer_ptr :
           bitstream_object.value[locations_field.c_str()].GetArray()) {
        bitstream_locations.push_back(location_integer_ptr.GetInt());
      }
      std::vector<int> capacity_values;
      for (const auto& capacity_integer_ptr :
           bitstream_object.value[capacity_field.c_str()].GetArray()) {
        capacity_values.push_back(capacity_integer_ptr.GetInt());
      }
      bitstream_parameters_map.insert({locations_field, bitstream_locations});
      bitstream_parameters_map.insert({capacity_field, capacity_values});
      bitstream_parameters_map.insert(
          {length_field,
           bitstream_object.value[length_field.c_str()].GetInt()});
      bitstream_parameters_map.insert(
          {resource_string_field,
           bitstream_object.value[resource_string_field.c_str()].GetString()});
      bitstream_parameters_map.insert(
          {is_backwards_field,
           bitstream_object.value[is_backwards_field.c_str()].GetInt()});
      operation_module_bitstream_map.insert(
          {bitstream_name, bitstream_parameters_map});
    }

    std::vector<std::vector<std::string>> start_locations_vector;
    for (const auto& start_location_array :
         operator_parameters[start_locations_field.c_str()].GetArray()) {
      std::vector<std::string> current_location_bitstreams;
      for (const auto& bitstream_name : start_location_array.GetArray()) {
        current_location_bitstreams.push_back(bitstream_name.GetString());
      }
      start_locations_vector.push_back(current_location_bitstreams);
    }
    resulting_string_map.insert(
        {operation_type,
         {operation_module_bitstream_map, start_locations_vector}});
  }
  return resulting_string_map;
}