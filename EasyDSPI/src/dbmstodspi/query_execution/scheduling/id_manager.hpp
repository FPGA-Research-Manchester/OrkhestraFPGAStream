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
#include <stack>
#include <vector>

#include "query_scheduling_data.hpp"

using easydspi::core_interfaces::query_scheduling_data::QueryNode;

namespace easydspi::dbmstodspi {

/**
 * @brief Class which assigns stream IDs to the input and output streams.
 */
class IDManager {
 private:
  /**
   * @brief Method for finding the index of an element in a vector.
   * @param vector Vector where the element should be found from.
   * @param element Element whose location in the vector we want to know.
   * @return Integer showing the element's location.
   */
  static auto FindStreamIndex(
      const std::vector<std::shared_ptr<QueryNode>> &stream_vector,
      const QueryNode &node) -> int;

  /**
   * @brief Allocate IDs which the input and output streams share.
   * @param current_node Current node which needs IDs.
   * @param current_node_input_ids Current input IDs vector.
   * @param output_ids All output IDs map.
   * @param current_node_output_ids Current output IDs vector.
   */
  static void AllocateInputIDs(
      const QueryNode &current_node, std::vector<int> &current_node_input_ids,
      std::map<std::string, std::vector<int>> &output_ids,
      std::vector<int> &current_node_output_ids,
      std::stack<int> &available_ids);
  /**
   * @brief Allocate IDs to output streams without IDs yet.
   * @param current_node Current node which needs to get more IDs
   * @param current_node_output_ids Vector of currently assigned output IDs.
   */
  static void AllocateLeftoverOutputIDs(
      const QueryNode &current_node, std::vector<int> &current_node_output_ids,
      std::stack<int> &available_ids);

  static auto SetUpAvailableIDs() -> std::stack<int>;

 public:
  /**
   * @brief Main method to start allocating IDs to all of the input and output
   * streams of the given nodes.
   * @param node_vector Vector of the nodes of the next FPGA run.
   * @param input_ids Map of all assigned input IDs.
   * @param output_ids Map of all assigned output IDs.
   */
  static void AllocateStreamIDs(
      const std::vector<QueryNode> &all_nodes,
      std::map<std::string, std::vector<int>> &input_ids,
      std::map<std::string, std::vector<int>> &output_ids);
};

}  // namespace easydspi::dbmstodspi