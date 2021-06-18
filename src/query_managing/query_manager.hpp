#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "config.hpp"
#include "memory_manager.hpp"
#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"

using dbmstodspi::input_managing::Config;
using dbmstodspi::data_managing::table_data::TableData;

namespace dbmstodspi::query_managing {

/**
 * @brief Class which contains the pipeline shown in the main README file. This
 * class processes query nodes and configures the FPGA according to the
 * specifications from the scheduler.
 */
class QueryManager {
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