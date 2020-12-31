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

class TableManager {
 public:
  static auto GetRecordSizeFromTable(const TableData& input_table) -> int;
  static void ReadInputTables(
      std::vector<StreamDataParameters>& input_stream_parameters,
      DataManager& data_manager,
      const std::vector<std::string>& stream_data_file_names,
      const std::vector<int>& stream_id_vector,
      std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks);
  static void ReadExpectedTables(
      std::vector<StreamDataParameters>& output_stream_parameters,
      DataManager& data_manager,
      const std::vector<std::string>& stream_data_file_names,
      const std::vector<int>& stream_id_vector,
      std::vector<std::unique_ptr<MemoryBlockInterface>>&
          allocated_memory_blocks,
      std::vector<TableData>& output_tables);
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
};