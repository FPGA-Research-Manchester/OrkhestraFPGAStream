#pragma once
#include <queue>
#include <set>
#include <utility>
#include <vector>

#include "operation_types.hpp"
#include "query_scheduling_data.hpp"

class NodeScheduler {
 public:
  static void FindAcceleratedQueryNodeSets(
      std::queue<std::pair<query_scheduling_data::ConfigurableModuleSet,
                           std::vector<query_scheduling_data::QueryNode>>>*
          accelerated_query_node_sets,
      std::vector<query_scheduling_data::QueryNode> starting_nodes);

 private:
  static auto IsOperatorSupported(
      operation_types::QueryOperation query_operation) -> bool;
  static auto IsModuleSetSupported(
      query_scheduling_data::ConfigurableModuleSet module_set) -> bool;
};