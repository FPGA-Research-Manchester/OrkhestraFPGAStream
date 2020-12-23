#pragma once
#include <string>
#include <vector>

#include "memory_manager.hpp"
#include "stream_data_parameters.hpp"
#include "table_data.hpp"
#include "query_scheduling_data.hpp"

class QueryManager {
 private:
  static void CheckTableData(const TableData& expected_table,
                             const TableData& resulting_table);

 public:
  static void RunQueries(std::vector<query_scheduling_data::QueryNode>);
};