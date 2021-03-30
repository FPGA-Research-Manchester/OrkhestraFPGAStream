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

  static void CheckElasticityNeeds(
      std::vector<StreamDataParameters> input_stream_parameters,
      operation_types::QueryOperation operation_type,
      std::vector<std::vector<int>> operation_parameters,
      query_scheduling_data::ConfigurableModulesVector loaded_modules);

  static auto IsMergeSortBigEnough(
      std::vector<StreamDataParameters> input_stream_parameters,
      std::vector<std::vector<int>> operation_parameters) -> bool;

 public:
  static void RunQueries(std::vector<query_scheduling_data::QueryNode>);
};