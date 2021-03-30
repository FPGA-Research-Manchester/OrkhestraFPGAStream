#pragma once
#include <string>
#include <utility>
#include <vector>

#include "memory_manager.hpp"
#include "query_scheduling_data.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"
#include "operation_types.hpp"

class QueryManager {
 private:
  static void CheckTableData(const TableData& expected_table,
                             const TableData& resulting_table);

  static auto GetBitstreamFileFromQueryNode(
      const std::pair<query_scheduling_data::ConfigurableModulesVector,
                      std::vector<query_scheduling_data::QueryNode>>&
          query_node) -> std::string;
  static auto GetModuleCountFromQueryNode(
      const std::pair<query_scheduling_data::ConfigurableModulesVector,
                      std::vector<query_scheduling_data::QueryNode>>&
          query_node) -> int;

 public:
  static void RunQueries(std::vector<query_scheduling_data::QueryNode>);
};