#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "table_data.hpp"

namespace dbmstodspi::data_managing {

/**
 * @brief Class to manage the data types and reading and writing of data to and
 * from CSV files.
 */
class DataManager {
 public:
  /**
   * @brief Constructor which need the config to setup the data types.
   * @param config_filename Path to the data types config file.
   */
  explicit DataManager(const std::map<std::string, double>& data_sizes)
      : data_type_sizes_(data_sizes){};
  /**
   * @brief Write data from the given CSV file to the TableData structure.
   * @param filename Path to the DBMS CSV data.
   * @return Information about the size and datatypes and also the data itself.
   */
  auto ParseDataFromCSV(const std::string& filename) -> TableData;
  /**
   * @brief Helper method to print table data for debugging.
   * @param table_data Data to be printed out.
   */
  static void PrintTableData(const TableData& table_data);

 private:
  /// Map to hold information about data type sizes from the given config file.
  std::map<std::string, double> data_type_sizes_;
  /**
   * @brief Read the data type configuration file to fill the #data_type_sizes
   * map.
   * @param config_filename Path to the data types configuration file.
   * @return A map which maps each datatype to its size modifier.
   */
  static auto GetDataConfiguration(const std::string& config_filename)
      -> std::map<std::string, double>;
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
      const std::vector<std::pair<std::string, int>>& data_types_vector);
  /**
   * @brief Helper method to print the string data from tables.
   * @param string_data Rows of different elements in string format.
   */
  static void PrintStringData(
      const std::vector<std::vector<std::string>>& string_data);
};

}  // namespace dbmstodspi::data_managing