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
  using MemoryReuseTargets = std::vector<std::pair<std::string, int>>;

  struct StreamResultParameters {
    int output_id;
    int memory_block_index;
    std::string filename;
    bool check_or_not;
  };

 private:
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
  static auto GetRecordSizeFromParameters(
      const DataManager& data_manager,
      std::vector<std::vector<int>> node_parameters, int stream_index) -> int;

  static void FindOutputNodes(
      std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
          scheduled_nodes,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links);

  static void AllocateOutputMemoryBlocks(
      fpga_managing::MemoryManager memory_manager,
      const DataManager& data_manager,
      std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          output_memory_blocks,
      const query_scheduling_data::QueryNode& node, std::vector<int>& row_count,
      std::vector<int>& record_sizes);
  static void AllocateInputMemoryBlocks(
      fpga_managing::MemoryManager memory_manager,
      const DataManager& data_manager,
      std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          input_memory_blocks,
      const query_scheduling_data::QueryNode& node, std::vector<int>& row_count,
      std::vector<int>& record_sizes);

  static auto CreateStreamParams(
      const DataManager& data_manager, const std::vector<int>& stream_ids,
      const std::vector<std::vector<int>>& node_parameters,
      const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          allocated_memory_blocks,
      const std::vector<int>& row_counts, const std::vector<int>& record_sizes)
      -> std::vector<fpga_managing::StreamDataParameters>;
  static void StoreStreamResultPrameters(
      std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters,
      const std::vector<int> stream_ids,
      const query_scheduling_data::QueryNode& node,
      const std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>&
          allocated_memory_blocks);
  static void ProcessResults(
      std::array<int, dbmstodspi::fpga_managing::query_acceleration_constants::
                          kMaxIOStreamCount>
          result_sizes,
      std::map<std::string, std::vector<StreamResultParameters>>&
          result_parameters);
  static void FreeMemoryBlocks(
      fpga_managing::MemoryManager memory_manager,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          input_memory_blocks,
      std::map<
          std::string,
          std::vector<std::unique_ptr<fpga_managing::MemoryBlockInterface>>>&
          output_memory_blocks,
      std::map<std::string, std::map<int, MemoryReuseTargets>>& reuse_links);

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