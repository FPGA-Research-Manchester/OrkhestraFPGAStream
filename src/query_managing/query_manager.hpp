#pragma once
#include <memory>
#include <string>
#include <vector>

#include "data_manager.hpp"
#include "fpga_manager.hpp"
#include "memory_block_interface.hpp"
#include "memory_manager.hpp"
#include "operation_types.hpp"
#include "stream_data_parameters.hpp"
#include "stream_initialisation_data.hpp"
#include "table_data.hpp"
#include "query_node.hpp"

class QueryManager {
 private:
  static void PrintInputDataOut(
      DataManager& data_manager,
      const std::vector<StreamInitialisationData>& input_data_locations);
  static auto GetRecordSizeFromTable(const TableData& input_table) -> int;
  static void CheckTableData(const TableData& expected_table,
                             const TableData& resulting_table);
  static void ReadOutputDataFromMemoryBlock(
      const std::unique_ptr<MemoryBlockInterface>& output_device,
      TableData& resulting_table, const int& result_size);
  static void WriteInputDataToMemoryBlock(
      const std::unique_ptr<MemoryBlockInterface>& input_device,
      const TableData& input_table);
  static void ReadInputTables(
      const std::vector<StreamInitialisationData>& input_data_locations,
      DataManager& data_manager,
      std::vector<StreamDataParameters>& input_streams);
  static void ReadExpectedTables(
      const std::vector<StreamInitialisationData>& output_data_locations,
      DataManager& data_manager, std::vector<TableData>& expected_output_tables,
      std::vector<StreamDataParameters>& output_streams,
      std::vector<TableData>& output_tables);
  static void RunQueryWithData(
      DataManager& data_manager, FPGAManager& fpga_manager,
      const std::vector<StreamInitialisationData>& input_data_locations,
      const std::vector<StreamInitialisationData>& output_data_locations,
      operation_types::QueryOperation operation);
  static void FillDataLocationsVector(
      std::vector<StreamInitialisationData>& data_locations,
      MemoryManager* memory_manager,
      const std::vector<std::string>& file_name_vector,
      const std::vector<int>& stream_id_vector);

 public:
  static void RunQueries(std::vector<QueryNode>);
};