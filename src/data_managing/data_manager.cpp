#include "data_manager.hpp"
#include "csv_reader.hpp"
#include "types_converter.hpp"
#include "config_reader.hpp"

#include <iostream>

auto DataManager::ParseDataFromCSV(std::string filename)
    -> TableData {
  std::vector<std::vector<std::string>> data_rows;
  std::vector<std::string> header_row;
  CSVReader::ReadTableData(filename, header_row, data_rows);

  std::vector<std::pair<std::string, int>> table_column_label_vector;
  for (auto column_data : header_row) {
    auto delimiter_position = column_data.find("-");
    auto name = column_data.substr(0, delimiter_position);
    auto value = column_data.substr(delimiter_position + 1);

    table_column_label_vector.push_back(
        std::make_pair(name, data_type_sizes_[name] * std::stoi(value)));
  }

  TableData table_data;
  table_data.table_column_label_vector = table_column_label_vector;
  TypesConverter::AddIntegerDataFromStringData(data_rows, table_data.table_data_vector, table_column_label_vector);
  return table_data;
}

auto DataManager::GetDataConfiguration(std::string config_filename)
    -> std::map<std::string, double> {
  return ConfigReader::ParseDataTypeSizesConfig(config_filename);
}

void DataManager::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& string_data,
    const std::vector<std::pair<std::string, int>> data_types_vector) {
  TypesConverter::AddStringDataFromIntegerData(integer_data, string_data,
                                               data_types_vector);
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

void DataManager::PrintTableData(const TableData table_data) {
  std::vector<std::vector<std::string>> string_data_vector;
  DataManager::AddStringDataFromIntegerData(table_data.table_data_vector,
                                            string_data_vector,
                                            table_data.table_column_label_vector);
  DataManager::PrintStringData(string_data_vector);
}
