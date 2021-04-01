#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "data_manager.hpp"
#include "memory_block_interface.hpp"
#include "memory_manager.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"
/**
 * Class to handle the input and output data, its parameters and tables.
 */
class TableManager {
 public:
  /**
   * Get the length of a record from the given table.
   * @param input_table Information about a data table.
   * @return Size of a record in the given table in 32 bit integers.
   */
  static auto GetRecordSizeFromTable(const TableData& input_table) -> int;
  /**
   * Read data from input fales into tables which will be in the DDR memory
   *  connected to the FPGA.
   * @param input_stream_parameters Data about the input streams which needs to
   *  get filled.
   * @param data_manager Class to deal with data type conversions.
   * @param stream_data_file_names Vector of input data files.
   * @param stream_id_vector Given IDs for the streams.
   * @param allocated_memory_blocks Vector of memory blocks mapped to DDR.
   * @param stream_specifications Query operation parameters which are connected
   *  with the given data.
   */
  static void ReadInputTables(
      std::vector<StreamDataParameters>& input_stream_parameters,
      DataManager& data_manager,
      const std::vector<std::string>& stream_data_file_names,
      const std::vector<int>& stream_id_vector,
      std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks,
      std::vector<std::vector<int>> stream_specifications);
  /**
   * Read data from the expected output tables for validity checks and output
   * stream parameters.
   * @param output_stream_parameters Vector where the output stream parameters
   * will be written to.
   * @param data_manager Class to deal with data type conversions.
   * @param stream_data_file_names Vector of expected output data files.
   * @param stream_id_vector Vector of given output IDs.
   * @param allocated_memory_blocks Vector of memory mapped memory blocks.
   * @param output_tables Vector of expected output data.
   * @param stream_specifications Query parameters to configure the output
   * streams.
   */
  static void ReadExpectedTables(
      std::vector<StreamDataParameters>& output_stream_parameters,
      DataManager& data_manager,
      const std::vector<std::string>& stream_data_file_names,
      const std::vector<int>& stream_id_vector,
      std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks,
      std::vector<TableData>& output_tables,
      std::vector<std::vector<int>> stream_specifications);
  /**
   * Read data from the memory mapped DDR after acceleration.
   * @param output_stream_parameters Query operation parameters for the output
   *  data.
   * @param output_tables Results of the query nodes' execution phase
   * @param result_record_counts How many records should be read from the DDR.
   * @param allocated_memory_blocks Vector of allocated memory blocks.
   */
  static void ReadResultTables(
      const std::vector<StreamDataParameters>& output_stream_parameters,
      std::vector<TableData>& output_tables,
      const std::vector<int>& result_record_counts,
      std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks);

 private:
  static void ReadOutputDataFromMemoryBlock(
      const std::unique_ptr<MemoryBlockInterface>& output_device,
      TableData& resulting_table, const int& result_size);
  static void WriteInputDataToMemoryBlock(
      const std::unique_ptr<MemoryBlockInterface>& input_device,
      const TableData& input_table);
  static void PrintWrittenData(
      std::string table_name,
      const std::unique_ptr<MemoryBlockInterface>& input_device,
      const TableData& input_table);
  static void PrintDataSize(const TableData& data_table);
};