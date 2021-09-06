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
#include <array>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "data_manager_interface.hpp"
#include "memory_block_interface.hpp"
#include "query_acceleration_constants.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"

using easydspi::core_interfaces::table_data::TableData;

namespace easydspi::dbmstodspi {

/**
 * @brief Class to handle the input and output data, its parameters and tables.
 */
class TableManager {
 public:
  /**
   * @brief Get the length of a record from the given table.
   * @param input_table Information about a data table.
   * @return Size of a record in the given table in 32 bit integers.
   */
  static auto GetRecordSizeFromTable(const TableData& input_table) -> int;
  /**
   * @brief Read data from the memory mapped DDR after acceleration.
   * @param output_stream_parameters Query operation parameters for the output
   * data.
   * @param output_tables Results of the query nodes' execution phase
   * @param result_record_counts How many records should be read from the DDR.
   * @param allocated_memory_blocks Vector of allocated memory blocks.
   */
  static void ReadResultTables(
      const std::vector<StreamDataParameters>& output_stream_parameters,
      std::vector<TableData>& output_tables,
      const std::array<int, query_acceleration_constants::kMaxIOStreamCount>&
          result_record_counts,
      std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks);

  /**
   * @brief Write the table to a CSV file with a timestamp
   * @param data_manager Data managing object to write CSV files with.
   * @param data_table Resulting data to be written to the file.
   * @param filename String of the name of the file to be written to.
   */
  static void WriteResultTableFile(const DataManagerInterface* data_manager,
                                   const TableData& data_table,
                                   const std::string& filename);

  /**
   * @brief Write data from file to given memory block.
   * @param data_manager Manager to handle data types.
   * @param stream_specification Config values describing table data types.
   * @param stream_index Stream index for getting the correct specification.
   * @param memory_device Memory block pointer.
   * @param filename CSV file.
   * @return How many rows of data were written and how big are they.
   */
  static auto WriteDataToMemory(
      const DataManagerInterface* data_manager,
      const std::vector<std::vector<int>>& stream_specification,
      int stream_index,
      const std::unique_ptr<MemoryBlockInterface>& memory_device,
      const std::string& filename) -> std::pair<int, int>;

  /**
   * @brief Get TableData object from the given memory pointer.
   * @param data_manager Manager to handle data types.
   * @param stream_specification Config values describing table data types.
   * @param stream_index Stream index for getting the correct specification.
   * @param memory_device Memory block pointer.
   * @param row_count How many rows of data should be read.
   * @return Table data.
   */
  static auto ReadTableFromMemory(
      const DataManagerInterface* data_manager,
      const std::vector<std::vector<int>>& stream_specification,
      int stream_index,
      const std::unique_ptr<MemoryBlockInterface>& memory_device, int row_count)
      -> TableData;
  /**
   * @brief Get TableData object from the given file.
   * @param data_manager Manager to handle data types.
   * @param stream_specification Config values describing table data types.
   * @param stream_index Stream index for getting the correct specification.
   * @param filename CSV file.
   * @return Table data.
   */
  static auto ReadTableFromFile(
      const DataManagerInterface* data_manager,
      const std::vector<std::vector<int>>& stream_specification,
      int stream_index, const std::string& filename) -> TableData;
  /**
   * @brief Get Column types and sizes from the input query parameters.
   * @param data_manager Manager to handle data types.
   * @param node_parameters Config values describing table data types.
   * @param stream_index Stream index for getting the correct specification.
   * @return Vector of column types and their sizes.
   */
  static auto GetColumnDefsVector(const DataManagerInterface* data_manager,
                                  std::vector<std::vector<int>> node_parameters,
                                  int stream_index)
      -> std::vector<std::pair<ColumnDataType, int>>;

 private:
  /**
   * @brief Read data from memory.
   * @param output_device Memory block.
   * @param resulting_table Table object with the column types.
   * @param result_size How many rows should be read.
   */
  static void ReadOutputDataFromMemoryBlock(
      const std::unique_ptr<MemoryBlockInterface>& output_device,
      TableData& resulting_table, const int& result_size);
  /**
   * @brief Write data from table object to memory.
   * @param input_device Memory block.
   * @param input_table Table object with the data.
   * @param previous_record_count How many records have been written to memory
   * already.
   */
  static void WriteInputDataToMemoryBlock(
      const std::unique_ptr<MemoryBlockInterface>& input_device,
      const TableData& input_table, int previous_record_count);
  /**
   * @brief Print data written to memory.
   * @param data_manager Data managing object to print data with.
   * @param table_name Name of the table.
   * @param input_device Memory pointer
   * @param input_table Table data holding column types information.
   */
  static void PrintWrittenData(
      const DataManagerInterface* data_manager, const std::string& table_name,
      const std::unique_ptr<MemoryBlockInterface>& input_device,
      const TableData& input_table);
  /**
   * @brief Method to print how big the table is.
   * @param data_table Data object.
   */
  static void PrintDataSize(const TableData& data_table);
  /**
   * @brief Get column data types without sizes.
   * @param stream_specification Input column types specification.
   * @param stream_index Stream index for getting the correct specification.
   * @return Vector of column types.
   */
  static auto GetColumnDataTypesFromSpecification(
      const std::vector<std::vector<int>>& stream_specification,
      int stream_index) -> std::vector<ColumnDataType>;
};

}  // namespace easydspi::dbmstodspi