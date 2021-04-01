#pragma once
#include <string>
#include <utility>
#include <vector>

#include "memory_manager.hpp"
#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"

/**
 * Class which contains the pipeline shown in the main README file. This class
 * processes query nodes and configures the FPGA according to the specifications
 * from the scheduler.
 */
class QueryManager {
 private:
  /**
   * Compare the accelerated query results with the expected data. Prints out
   *  tables if they are different.
   *
   * @param expected_table Given expected data
   * @param expected_table Data which was output by the FPGA.
   */
  static void CheckTableData(const TableData& expected_table,
                             const TableData& resulting_table);

  /**
   * Check the module library to see which bitstream should get loaded on the
   *  FPGA.
   *
   * @param query_node Information about the collection of nodes selected for
   * the next run.
   */
  static auto GetBitstreamFileFromQueryNode(
      const std::pair<query_scheduling_data::ConfigurableModulesVector,
                      std::vector<query_scheduling_data::QueryNode>>&
          query_node) -> std::string;

  /**
   * Count how many modules will be loaded with the given collection of nodes to
   *  be accelerated.
   *
   * @param query_node Information about the collection of nodes selected for
   * the next run.
   */
  static auto GetModuleCountFromQueryNode(
      const std::pair<query_scheduling_data::ConfigurableModulesVector,
                      std::vector<query_scheduling_data::QueryNode>>&
          query_node) -> int;

 public:
  /**
   * This is the main method which gets called to run the software to accelerate
   *  the given queries. This method will call the scheduler and data manager to
   *  eventually call the fpga manager to accelerate the desired query nodes.
   *
   * @param starting_query_nodes Vector of nodes from which the parsing starts.
   */
  static void RunQueries(
      std::vector<query_scheduling_data::QueryNode> starting_query_nodes);
};