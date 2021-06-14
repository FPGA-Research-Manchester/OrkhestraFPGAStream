#include "data_manager.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "config_reader.hpp"
#include "csv_reader.hpp"
#include "logger.hpp"
#include "types_converter.hpp"

using namespace dbmstodspi::data_managing;

auto DataManager::ParseDataFromCSV(const std::string& filename) -> TableData {
  std::vector<std::vector<std::string>> data_rows;
  std::vector<std::string> header_row;
  CSVReader::ReadTableData(filename, header_row, data_rows);
  int size = 0;
  std::vector<std::pair<std::string, int>> table_column_label_vector;
  for (const auto& column_data : header_row) {
    auto delimiter_position = column_data.find('-');
    auto name = column_data.substr(0, delimiter_position);
    auto value = column_data.substr(delimiter_position + 1);

    double data_type_size = data_type_sizes_[name] * std::stoi(value);

    if (data_type_size != static_cast<int>(data_type_size)) {
      throw std::runtime_error(value + "size is not supported!");
    }

    table_column_label_vector.emplace_back(name,
                                           static_cast<int>(data_type_size));
    size += static_cast<int>(data_type_size);
  }

  TableData table_data;
  table_data.table_column_label_vector = table_column_label_vector;
  table_data.table_data_vector.reserve(size * data_rows.size());
  TypesConverter::AddIntegerDataFromStringData(
      data_rows, table_data.table_data_vector, table_column_label_vector);
  return table_data;
}

auto DataManager::GetDataConfiguration(const std::string& config_filename)
    -> std::map<std::string, double> {
  return ConfigReader::ParseDataTypeSizesConfig(config_filename);
}

void DataManager::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& string_data,
    const std::vector<std::pair<std::string, int>>& data_types_vector) {
  TypesConverter::AddStringDataFromIntegerData(integer_data, string_data,
                                               data_types_vector);
}

void DataManager::PrintStringData(
    const std::vector<std::vector<std::string>>& string_data) {
  auto log_level = dbmstodspi::logger::LogLevel::kDebug;
  if (dbmstodspi::logger::ShouldLog(log_level)) {
    std::stringstream ss;
    for (const auto& row : string_data) {
      for (const auto& element : row) {
        if (&element != &row.back()) {
          ss << element << ",";
        } else {
          ss << element << std::endl;
        }
      }
    }
    ss << std::endl;
    dbmstodspi::logger::Log(log_level, ss.str());
  }
}

void DataManager::PrintTableData(const TableData& table_data) {
  std::vector<std::vector<std::string>> string_data_vector;
  DataManager::AddStringDataFromIntegerData(
      table_data.table_data_vector, string_data_vector,
      table_data.table_column_label_vector);
  DataManager::PrintStringData(string_data_vector);
}
