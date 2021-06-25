#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.hpp"
#include "data_manager.hpp"
#include "memory_manager.hpp"
#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"

using dbmstodspi::data_managing::DataManager;
using dbmstodspi::data_managing::table_data::TableData;
using dbmstodspi::input_managing::Config;

namespace dbmstodspi::query_managing {
/**
 * @brief Class which contains the pipeline shown in the main README file. This
 * class processes query nodes and configures the FPGA according to the
 * specifications from the scheduler.
 */
class QueryManager {
  using RecordSizeAndCount = std::pair<int, int>;
  using MemoryReuseTargets = std::vector<std::pair<std::string, int>>;

 private:
  struct StreamResultParameters {
    /// For memory block and stream size access which is stored after the run
    int stream_index;
    int output_id;
    std::string filename;
    bool check_results;
    std::vector<std::vector<int>> stream_specifications;

    StreamResultParameters(
        int stream_index, int output_id, std::string filename,
        bool check_results,
        const std::vector<std::vector<int>>& stream_specifications)
        : stream_index{stream_index},
          output_id{output_id},
          filename{filename},
          check_results{check_results},
          stream_specifications{stream_specifications} {};
  };

  /**
   * @brief Compare the accelerated query results with the expected data.
   *
   * Prints out tables if they are different.
   * @param expected_table Given expected data
   * @param expected_table Data which was output by the FPGA.
   */
  static void CheckTableData(const TableData& expected_table,
                             const TableData& resulting_table);

  static void InitialiseMemoryBlockVector(
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          memory_blocks,
      int stream_count, std::string node_name);
  static void InitialiseStreamSizeVector(
      std::map<std::string, std::vector<RecordSizeAndCount>>& stream_sizes,
      int stream_count, std::string node_name);
  static auto GetRecordSizeFromParameters(
      const DataManager& data_manager,
      const std::vector<std::vector<int>>& node_parameters, int stream_index)
      -> int;

  static void FindOutputNodes(
      const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>&
          scheduled_nodes,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links);

  static void AllocateOutputMemoryBlocks(
      fpga_managing::MemoryManager& memory_manager,
      const DataManager& data_manager,
      std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          output_memory_blocks,
      const query_scheduling_data::QueryNode& node,
      std::vector<RecordSizeAndCount>& output_stream_sizes);
  static void AllocateInputMemoryBlocks(
      fpga_managing::MemoryManager& memory_manager,
      const DataManager& data_manager,
      std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          input_memory_blocks,
      const query_scheduling_data::QueryNode& node,
      const std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      std::vector<RecordSizeAndCount>& input_stream_sizes);

  static auto CreateStreamParams(
      const std::vector<int>& stream_ids,
      const std::vector<std::vector<int>>& node_parameters,
      const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          allocated_memory_blocks,
      const std::vector<RecordSizeAndCount>& stream_sizes)
      -> std::vector<fpga_managing::StreamDataParameters>;
  static void StoreStreamResultPrameters(
      std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<int>& stream_ids,
      const query_scheduling_data::QueryNode& node,
      const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          allocated_memory_blocks);
  static void ProcessResults(
      const DataManager& data_manager,
      const std::array<int, dbmstodspi::fpga_managing::
                                query_acceleration_constants::kMaxIOStreamCount>
          result_sizes,
      const std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          allocated_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes);
  static void FreeMemoryBlocks(
      fpga_managing::MemoryManager& memory_manager,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          input_stream_sizes,
      std::map<std::string, std::vector<RecordSizeAndCount>>&
          output_stream_sizes,
      std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links);
  static void CheckResults(
      const DataManager& data_manager,
      const std::unique_ptr<fpga_managing::MemoryBlockInterface>& memory_device,
      int row_count, std::string filename,
      const std::vector<std::vector<int>>& node_parameters, int stream_index);
  static void WriteResults(
      const DataManager& data_manager,
      const std::unique_ptr<fpga_managing::MemoryBlockInterface>& memory_device,
      int row_count, std::string filename,
      const std::vector<std::vector<int>>& node_parameters, int stream_index);
  static void CopyMemoryData(
      int table_size,
      const std::unique_ptr<fpga_managing::MemoryBlockInterface>&
          source_memory_device,
      const std::unique_ptr<fpga_managing::MemoryBlockInterface>&
          target_memory_device);

 public:
  /**
   * @brief This is the main method which gets called to run the software to
   * accelerate the given queries.
   *
   * This method will call the scheduler and data manager to eventually call the
   * fpga manager to accelerate the desired query nodes.
   * @param starting_query_nodes Vector of nodes from which the parsing starts.
   * @param config Config struct which holds all of the corresponding hardware
   * modules and drivers and configuration settings.
   */
  static void RunQueries(
      std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
          starting_query_nodes,
      const Config& config);
};

}  // namespace dbmstodspi::query_managing