#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "table_data.hpp"

using dbmstodspi::data_managing::table_data::ColumnDataType;
using dbmstodspi::data_managing::table_data::TableData;

namespace dbmstodspi::data_managing {

/**
 * @brief Class to manage the data types and reading and writing of data to and
 * from CSV files.
 */
class DataManager {
 public:
  /**
   * @brief Constructor to setup data types size scale values.
   * @param data_sizes Data type size scaling values.
   */
  explicit DataManager(std::map<ColumnDataType, double> data_sizes)
      : data_type_sizes_(std::move(data_sizes)){};
  /**
   * @brief Write data from the given CSV file to the TableData structure.
   * @param filename Path to the DBMS CSV data.
   * @param column_data_types Vector of data type enums for each column of data in the table.
   * @param column_sizes Vector of column sizes.
   * @return Information about the size and datatypes and also the data itself.
   */
  auto ParseDataFromCSV(const std::string& filename,
                        const std::vector<ColumnDataType>& column_data_types,
                        const std::vector<int>& column_sizes) -> TableData;
  /**
   * @brief Helper method to print table data for debugging.
   * @param table_data Data to be printed out.
   */
  static void PrintTableData(const TableData& table_data);

 private:
  /// Map to hold information about data type sizes from the given config file.
  std::map<ColumnDataType, double> data_type_sizes_;
  /**
   * @brief Helper method for printing out table data.
   *
   * Convert the integers in the table data to printable strings using
   * TypesConverter::AddStringDataFromIntegerData
   * @param integer_data Input vector of data in integer format.
   * @param string_data Output vector of data in string format.
   * @param data_types_vector Information about the column data types.
   */
  static void AddStringDataFromIntegerData(
      const std::vector<uint32_t>& integer_data,
      std::vector<std::vector<std::string>>& string_data,
      const std::vector<std::pair<ColumnDataType, int>>& data_types_vector);
  /**
   * @brief Helper method to print the string data from tables.
   * @param string_data Rows of different elements in string format.
   */
  static void PrintStringData(
      const std::vector<std::vector<std::string>>& string_data);
};

}  // namespace dbmstodspi::data_managing