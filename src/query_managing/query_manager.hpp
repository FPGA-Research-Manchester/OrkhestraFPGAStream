#pragma once
#include <string>
#include <utility>
#include <vector>

#include "memory_manager.hpp"
#include "operation_types.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"

namespace dbmstodspi {
namespace query_managing {

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
  static void CheckTableData(const data_managing::TableData& expected_table,
                             const data_managing::TableData& resulting_table);

 public:
  /**
   * @brief This is the main method which gets called to run the software to
   * accelerate the given queries.
   *
   * This method will call the scheduler and data manager to eventually call the
   * fpga manager to accelerate the desired query nodes.
   * @param starting_query_nodes Vector of nodes from which the parsing starts.
   */
  static void RunQueries(
      std::vector<query_scheduling_data::QueryNode> starting_query_nodes);
};

}  // namespace query_managing
}  // namespace dbmstodspi