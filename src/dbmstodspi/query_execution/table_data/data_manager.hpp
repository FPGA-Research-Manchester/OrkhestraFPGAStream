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

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "data_manager_interface.hpp"

using orkhestrafs::core_interfaces::table_data::ColumnDataType;
using orkhestrafs::core_interfaces::table_data::TableData;
using orkhestrafs::dbmstodspi::MemoryBlockInterface;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to manage the data types and reading and writing of data to and
 * from CSV files.
 */
class DataManager : public DataManagerInterface {
 public:
  ~DataManager() override = default;

  /**
   * @brief Constructor to setup data types size scale values.
   * @param data_sizes Data type size scaling values.
   * @param spearator Which character is used for separating columns.
   */
  explicit DataManager(std::map<ColumnDataType, double> data_sizes,
                       char separator)
      : data_type_sizes_(std::move(data_sizes)), separator_(separator){};
  /**
   * @brief Write data from the given CSV file to the TableData structure in
   * chunks.
   * @param filename Path to the DBMS CSV data.
   * @param column_data_types Vector of data type enums for each column of data
   * in the table.
   * @param column_sizes Vector of column sizes.
   * @param rows_already_read How many rows have been parsed already.
   * @return Information about the size and datatypes and also the data itself.
   */
  auto ParseDataFromCSV(const std::string& filename,
                        const std::vector<ColumnDataType>& column_data_types,
                        const std::vector<int>& column_sizes,
                        int& rows_already_read) const -> TableData override;

  /**
   * @brief Write data dirtly from files to memory blocks.
   * @param filename CSV file.
   * @param column_defs_vector Vector defining column types and sizes.
   * @param memory_device Memory block pointer.
   * @return How many rows were written.
   */
  auto WriteDataFromCSVToMemory(
      const std::string& filename,
      const std::vector<std::pair<ColumnDataType, int>>& column_defs_vector,
      const std::unique_ptr<MemoryBlockInterface>& memory_device) const
      -> int override;

  /**
   * @brief
   * @param table_column_defs
   * @param filename
   * @return
   */
  auto ReadIntegerDataFromCSV(
      const std::vector<std::pair<ColumnDataType, int>>& table_column_defs,
      const std::string& filename) const -> std::vector<uint32_t> override;

  /**
   * @brief Calculate column sizes given types and configured scales.
   * @param column_data_types Column types.
   * @param column_sizes Column scales.
   * @return Vector of column type and size information.
   */
  auto GetHeaderColumnVector(
      const std::vector<ColumnDataType>& column_data_types,
      const std::vector<int>& column_sizes) const
      -> std::vector<std::pair<ColumnDataType, int>> override;

  /**
   * @brief Helper method to print table data for debugging.
   * @param table_data Data to be printed out.
   */
  void PrintTableData(const TableData& table_data) const override;

  /**
   * @brief Method to write the table into a file with the given filename.
   * @param table_data Data to be written to the file.
   * @param filename Name of the file to be written to.
   */
  void WriteTableData(const TableData& table_data,
                      const std::string& filename) const override;

 private:
  /// Which char is used to separate columns.
  char separator_;
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
  void PrintStringData(
      const std::vector<std::vector<std::string>>& string_data) const;
};

}  // namespace orkhestrafs::dbmstodspi