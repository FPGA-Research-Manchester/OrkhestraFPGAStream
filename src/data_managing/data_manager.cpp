#include "data_manager.hpp"
#include "csv_reader.hpp"
#include "types_converter.hpp"
#include "config_reader.hpp"

#include <iostream>

auto DataManager::GetDataConfiguration(std::string config_filename)
    -> std::map<std::string, double> {
  return ConfigReader::ParseDataTypeSizesConfig(config_filename);
}

void DataManager::AddStringDataFromCSV(
    const std::string& file_name,
    std::vector<std::vector<std::string>>& resulting_strings) {
  CSVReader::AddCSVRows(file_name, resulting_strings);
}

void DataManager::AddIntegerDataFromStringData(
    const std::vector<std::vector<std::string>>& string_data,
    std::vector<uint32_t>& integer_data) {
  TypesConverter::AddIntegerDataFromStringData(string_data, integer_data);
}

void DataManager::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& string_data,
    const std::vector<int>& data_type_sizes) {
  TypesConverter::AddStringDataFromIntegerData(integer_data, string_data, data_type_sizes);
}

void DataManager::PrintStringData(
    const std::vector<std::vector<std::string>>& string_data) {
  for (const auto& row : string_data) {
    for (const auto& element : row) {
      std::cout << element << " ";
    }
    std::cout << std::endl;
  }
}
