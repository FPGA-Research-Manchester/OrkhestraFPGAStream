#include <cstdio>
#include <stdexcept>

#include "rapidjson_reader.hpp"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
 
using rapidjson::Document;
using rapidjson::FileReadStream;
using easydspi::dbmstodspi::RapidJSONReader;

std::unique_ptr<Document> RapidJSONReader::read(std::string json_filename) {
  FILE* file_pointer = fopen(json_filename.c_str(), "rb");  // non-Windows use "r"

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

std::map<std::string, std::string>
easydspi::dbmstodspi::RapidJSONReader::readDriverLibrary(
    std::string json_filename) {
  const auto document = read(json_filename);
  std::map<std::string, std::string> value_map;
  for (const auto& element : document->GetObject()) {
    value_map.insert({element.name.GetString(), element.value.GetString()});
  }
  return value_map;
}

std::map<std::string, double>
easydspi::dbmstodspi::RapidJSONReader::readDataSizes(
    std::string json_filename) {
  const auto document = read(json_filename);
  std::map<std::string, double> value_map;
  for (const auto& element : document->GetObject()) {
    value_map.insert({element.name.GetString(), element.value.GetDouble()});
  }
  return value_map;
}

std::map<std::string, int>
easydspi::dbmstodspi::RapidJSONReader::readReqMemorySpace(
    std::string json_filename) {
  const auto document = read(json_filename);
  std::map<std::string, int> value_map;
  for (const auto& element : document->GetObject()) {
    value_map.insert({element.name.GetString(), element.value.GetInt()});
  }
  return value_map;
}

std::map<std::vector<std::pair<std::string, std::vector<int>>>, std::string>
easydspi::dbmstodspi::RapidJSONReader::readAcceleratorLibrary(
    std::string json_filename) {
  std::string module_combination_names_field = "module_combinations";
  std::string accelerator_name_field = "accelerators";
  std::string module_name_field = "name";
  std::string module_capacity_field = "capacity";
  const auto document = read(json_filename);
  auto document_ptr = document.get();
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
