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
  static auto IsModuleSetSupported(
      query_scheduling_data::ConfigurableModuleSet module_set) -> bool;
  static auto IsNodeIncluded(
      std::vector<query_scheduling_data::QueryNode> node_vector,
      query_scheduling_data::QueryNode searched_node) -> bool;
  static auto IsNodeAvailable(
      std::vector<query_scheduling_data::QueryNode> scheduled_nodes,
      query_scheduling_data::QueryNode current_node) -> bool;
  static auto FindModuleSize(
      operation_types::QueryOperation query_operation,
      query_scheduling_data::ConfigurableModuleSet current_set) -> int;
  static auto FindNextAvailableNode(
      std::vector<query_scheduling_data::QueryNode>& already_scheduled_nodes,
      std::vector<query_scheduling_data::QueryNode>& starting_nodes)
      -> std::vector<query_scheduling_data::QueryNode>::iterator;
  static void CheckNodeForModuleSet(
      std::vector<query_scheduling_data::QueryNode>::iterator &iterator,
      query_scheduling_data::ConfigurableModuleSet &current_set,
      std::vector<query_scheduling_data::QueryNode> &current_query_nodes,
      std::vector<query_scheduling_data::QueryNode> &scheduled_queries,
      std::vector<query_scheduling_data::QueryNode> &starting_nodes);
  // Not used
  static auto IsOperatorSupported(
      operation_types::QueryOperation query_operation) -> bool;
};