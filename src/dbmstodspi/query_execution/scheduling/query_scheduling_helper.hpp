/*
Copyright 2021 University of Manchester

Licensed under the Apache License, Version 2.0(the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http:  // www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#include "query_scheduling_data.hpp"
#include "scheduling_query_node.hpp"
#include "table_data.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;
using orkhestrafs::core_interfaces::table_data::TableMetadata;
using orkhestrafs::dbmstodspi::SchedulingQueryNode;

namespace orkhestrafs::dbmstodspi {

/**
 * @brief Class to contain static helper methods.
 */
class QuerySchedulingHelper {
 public:
  /**
   * @brief Find the index of the current node of the previous node's next
   * nodes.
   * @param current_node Current node which's index is unknown
   * @param previous_node Previous node.
   * @return Integer showing which stream of the previous node is used for this
   * node.
   */
  static auto FindNodePtrIndex(QueryNode* current_node,
                               QueryNode* previous_node) -> int;

  /**
   * @brief Method to check if the table is sorted.
   * @param table_data Table metadata
   * @return Boolean flag checking if there is only one sorted sequence covering
   * the whole table.
   */
  static auto IsTableSorted(TableMetadata table_data) -> bool;

  /**
   * @brief Method to find new available nodes
   * @param node_name Currently scheduled nodes
   * @param past_nodes All of the nodes which have been processed allready
   * @param graph All nodes
   * @return Vector of node names which are now available.
   */
  static auto GetNewAvailableNodesAfterSchedulingGivenNode(
      std::string node_name, const std::vector<std::string>& past_nodes,
      const std::map<std::string, SchedulingQueryNode>& graph)
      -> std::vector<std::string>;

  /**
   * @brief Move current table names to the next nodes.
   * @param graph All nodes
   * @param node_name Current node name
   * @param table_names Table name vector to add to next nodes.
   */
  static void AddNewTableToNextNodes(
      std::map<std::string, SchedulingQueryNode>& graph, std::string node_name,
      const std::vector<std::string>& table_names);

  /**
   * @brief Method to get current node index from the next nodes perspective and
   * the index of the output stream.
   * @param graph All nodes
   * @param next_node_name Next node
   * @param current_node_name Current node
   * @return Vector of node and stream indexes to get from the next node to the
   * current node.
   */
  static auto GetCurrentNodeIndexesByName(
      const std::map<std::string, SchedulingQueryNode>& graph,
      std::string next_node_name, std::string current_node_name)
      -> std::vector<std::pair<int, int>>;
};

}  // namespace orkhestrafs::dbmstodspi