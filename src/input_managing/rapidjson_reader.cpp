#include "rapidjson_reader.hpp"

#include <cstdio>
#include <stdexcept>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"

using dbmstodspi::input_managing::RapidJSONReader;
using rapidjson::Document;
using rapidjson::FileReadStream;

std::unique_ptr<Document> RapidJSONReader::read(std::string json_filename) {
  FILE* file_pointer =
      fopen(json_filename.c_str(), "r");

  if (!file_pointer) {
    throw std::runtime_error("Couldn't find: " + json_filename);
  }

  char readBuffer[8192];
  FileReadStream input_stream(file_pointer, readBuffer, sizeof(readBuffer));

  auto document = std::make_unique<Document>();
  document->ParseStream(input_stream);

  fclose(file_pointer);
  return document;
}

std::map<std::string, RapidJSONReader::InputNodeParameters>
RapidJSONReader::ReadInputDefinition(std::string json_filename) {
  const auto document = read(json_filename);
  std::map<std::string, RapidJSONReader::InputNodeParameters> input_node_map;
  for (const auto& node : document->GetObject()) {
    InputNodeParameters node_parameters_map;
    for (const auto& node_parameter : node.value.GetObject()) {
      if (!node_parameter.value.IsNull()) {
        if (node_parameter.value.IsString()) {
          node_parameters_map.insert({node_parameter.name.GetString(),
                                      node_parameter.value.GetString()});
        } else if (node_parameter.value.IsArray()) {
          std::vector<std::string> string_array;
          for (const auto& value_array_string :
               node_parameter.value.GetArray()) {
            if (value_array_string.IsString()) {
              string_array.push_back(value_array_string.GetString());
            } else {
              string_array.push_back("");
            }
          }
          node_parameters_map.insert(
              {node_parameter.name.GetString(), string_array});
        } else if (node_parameter.value.IsObject()) {
          std::map<std::string, std::vector<std::vector<int>>>
              operation_parameters_map;
          for (const auto& operation_parameter :
               node_parameter.value.GetObject()) {
            std::vector<std::vector<int>> parameter_vectors;
            for (const auto& given_parameter_vectors :
                 operation_parameter.value.GetArray()) {
              std::vector<int> parameters;
              if (given_parameter_vectors.Size() != 0 && given_parameter_vectors[0]
                      .IsString()) {
                parameters = ConvertCharStringToAscii(
                    given_parameter_vectors[0].GetString(),
                    given_parameter_vectors[1].GetInt());
              } else {
                for (const auto& parameter :
                     given_parameter_vectors.GetArray()) {
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