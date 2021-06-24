#include "data_manager.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "csv_reader.hpp"
#include "logger.hpp"
#include "types_converter.hpp"
#include "util.hpp"

using namespace dbmstodspi::data_managing;

using dbmstodspi::data_managing::table_data::ColumnDataType;
using dbmstodspi::data_managing::table_data::TableData;
using dbmstodspi::util::IsValidFile;

auto DataManager::ReadIntegerDataFromCSV(
    const std::vector<std::pair<ColumnDataType, int>> table_column_defs,
    const std::string& filename) -> std::vector<uint32_t> {
  if (IsValidFile(filename)) {
    auto data_rows = CSVReader::ReadTableData(filename);
    std::vector<uint32_t> table_data_vector;
    TypesConverter::AddIntegerDataFromStringData(data_rows, table_data_vector,
                                                 table_column_defs);
    return table_data_vector;
  } else {
    throw std::runtime_error("No file " + filename + " found!");
  }
}

auto DataManager::GetHeaderColumnVector(
    const std::vector<ColumnDataType>& column_data_types,
    const std::vector<int>& column_sizes) const
    -> std::vector<std::pair<ColumnDataType, int>> {
  std::vector<std::pair<ColumnDataType, int>> table_column_label_vector;
  // Assuming data type vector and column size vectors are the same length
  for (int i = 0; i < column_data_types.size(); i++) {
    double data_type_size =
        data_type_sizes_.at(column_data_types.at(i)) * column_sizes.at(i);
    if (data_type_size != static_cast<int>(data_type_size)) {
      throw std::runtime_error(column_sizes.at(i) + "size is not supported!");
    }

    table_column_label_vector.emplace_back(column_data_types.at(i),
                                           static_cast<int>(data_type_size));
  }

  return table_column_label_vector;
}

auto DataManager::ParseDataFromCSV(
    const std::string& filename,
    const std::vector<ColumnDataType>& column_data_types,
    const std::vector<int>& column_sizes) const -> TableData {
  int size = 0;
  std::vector<std::pair<ColumnDataType, int>> table_column_label_vector;
  // Assuming data type vector and column size vectors are the same length
  for (int i = 0; i < column_data_types.size(); i++) {
    double data_type_size =
        data_type_sizes_.at(column_data_types.at(i)) * column_sizes.at(i);
    if (data_type_size != static_cast<int>(data_type_size)) {
      throw std::runtime_error(column_sizes.at(i) + "size is not supported!");
    }

    table_column_label_vector.emplace_back(column_data_types.at(i),
                                           static_cast<int>(data_type_size));
    size += static_cast<int>(data_type_size);
  }

  TableData table_data;
  table_data.table_column_label_vector = table_column_label_vector;

  if (IsValidFile(filename)) {
    auto data_rows = CSVReader::ReadTableData(filename);
    table_data.table_data_vector.reserve(size * data_rows.size());
    TypesConverter::AddIntegerDataFromStringData(
        data_rows, table_data.table_data_vector, table_column_label_vector);
  }

  return table_data;
}

void DataManager::AddStringDataFromIntegerData(
    const std::vector<uint32_t>& integer_data,
    std::vector<std::vector<std::string>>& string_data,
    const std::vector<std::pair<ColumnDataType, int>>& data_types_vector) {
  TypesConverter::AddStringDataFromIntegerData(integer_data, string_data,
                                               data_types_vector);
}

void DataManager::PrintStringData(
    const std::vector<std::vector<std::string>>& string_data) {
  auto log_level = dbmstodspi::logger::LogLevel::kDebug;
  if (dbmstodspi::logger::ShouldLog(log_level)) {
    std::stringstream ss;
    ss << "data: " << std::endl;
    for (const auto& row : string_data) {
      for (const auto& element : row) {
        if (&element != &row.back()) {
          ss << element << ",";
        } else {
          ss << element << std::endl;
        }
      }
    }
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

void DataManager::WriteTableData(const TableData& table_data,
                                 std::string filename) {
  std::vector<std::vector<std::string>> string_data_vector;
  DataManager::AddStringDataFromIntegerData(
      table_data.table_data_vector, string_data_vector,
      table_data.table_column_label_vector);
  std::ofstream output_file(filename, std::ios::trunc);
  if (output_file.is_open()) {
    for (const auto& line : string_data_vector) {
      for (auto iter = line.begin(); iter != line.end(); iter++) {
        if (iter != line.begin()) output_file << ",";
        output_file << *iter;
      }
      output_file << std::endl;
    }
    output_file.close();
  } else {
    throw std::runtime_error("Can't open " + filename);
  }
}
