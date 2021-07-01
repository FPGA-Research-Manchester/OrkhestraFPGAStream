#pragma once
#include <stack>
#include <vector>

#include "query_scheduling_data.hpp"

namespace dbmstodspi::query_managing {

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
      const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
          &stream_vector,
      const query_scheduling_data::QueryNode &node) -> int;

  /**
   * @brief Allocate IDs which the input and output streams share.
   * @param current_node Current node which needs IDs.
   * @param current_node_input_ids Current input IDs vector.
   * @param output_ids All output IDs map.
   * @param current_node_output_ids Current output IDs vector.
   */
  static void AllocateInputIDs(
      const query_scheduling_data::QueryNode &current_node,
      std::vector<int> &current_node_input_ids,
      std::map<std::string, std::vector<int>> &output_ids,
      std::vector<int> &current_node_output_ids, std::stack<int>& available_ids);
  /**
   * @brief Allocate IDs to output streams without IDs yet.
   * @param current_node Current node which needs to get more IDs
   * @param current_node_output_ids Vector of currently assigned output IDs.
   */
  static void AllocateLeftoverOutputIDs(
      const query_scheduling_data::QueryNode &current_node,
      std::vector<int> &current_node_output_ids, std::stack<int>& available_ids);

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
      const std::vector<query_scheduling_data::QueryNode> &all_nodes,
      std::map<std::string, std::vector<int>> &input_ids,
      std::map<std::string, std::vector<int>> &output_ids);
};

}  // namespace dbmstodspi::query_managing