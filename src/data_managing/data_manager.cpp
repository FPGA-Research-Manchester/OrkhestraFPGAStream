#include "data_manager.hpp"
#include "csv_reader.hpp"
#include "types_converter.hpp"

#include <iostream>

void DataManager::AddStringDataFromCSV(
    std::string file_name, std::vector<std::vector<std::string>>& resulting_strings) {
  CSVReader::AddCSVRows(file_name, resulting_strings);
}

void DataManager::AddIntegerDataFromStringData(
    std::vector<std::vector<std::string>> string_data, std::vector<uint32_t>& integer_data) {
  DataArraysConverter::AddIntegerDataFromStringData(string_data, integer_data);
}

void DataManager::AddStringDataFromIntegerData(
    std::vector<uint32_t> integer_data, std::vector<std::vector<std::string>> & string_data, std::vector<int> data_type_sizes) {
  DataArraysConverter::AddStringDataFromIntegerData(integer_data, string_data, data_type_sizes);
}

void DataManager::PrintStringData(std::vector<std::vector<std::string>> string_data) {
  for (auto row : string_data) {
    for (auto element : row) {
      std::cout << element << " ";
    }
    std::cout << std::endl;
  }
}
