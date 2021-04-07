#pragma once
#include <stack>
#include <vector>

#include "query_scheduling_data.hpp"

namespace dbmstodspi {
namespace query_managing {

/**
 * @brief Class which assigns stream IDs to the input and output streams.
 */
class IDManager {
 private:
  /// Keep track of available IDs
  std::stack<int> available_ids_{};
  /**
   * @brief Generic method for finding the index of an element in a vector.
   * @tparam T Type of the elements in the vector.
   * @param vector Vector where the element should be found from.
   * @param element Element whose location in the vector we want to know.
   * @return Integer showing the element's location.
   */
  template <typename T>
  auto FindElementIndex(const std::vector<T> &vector, const T &element) -> int;

  /**
   * @brief Allocate IDs which the input and output streams share.
   * @param current_node Current node which needs IDs.
   * @param all_nodes Collection of all nodes to find already assigned IDs.
   * @param current_node_input_ids Current input IDs vector.
   * @param output_ids All output IDs vector.
   * @param current_node_output_ids Current output IDs vector.
   */
  void AllocateInputIDs(
      const query_scheduling_data::QueryNode &current_node,
      const std::vector<query_scheduling_data::QueryNode> &all_nodes,
      std::vector<int> &current_node_input_ids,
      std::vector<std::vector<int>> &output_ids,
      std::vector<int> &current_node_output_ids);
  /**
   * @brief Allocate IDs to output streams without IDs yet.
   * @param current_node Current node which needs to get more IDs
   * @param current_node_output_ids Vector of currently assigned output IDs.
   */
  void AllocateLeftoverOutputIDs(
      const query_scheduling_data::QueryNode &current_node,
      std::vector<int> &current_node_output_ids);

 public:
  /**
   * @brief Constructor for IDManager to setup the available IDs stack.
   */
  IDManager();
  /**
   * @brief Make an used ID available again.
   * @param available_id ID which can be made available.
   */
  void MakeIDAvailable(int available_id);
  /**
   * @brief Main method to start allocating IDs to all of the input and output
   * streams of the given nodes.
   * @param node_vector Vector of the nodes of the next FPGA run.
   * @param input_ids Vector of all assigned input IDs.
   * @param output_ids Vector of all assigned output IDs.
   */
  void AllocateStreamIDs(
      std::vector<query_scheduling_data::QueryNode> node_vector,
      std::vector<std::vector<int>> &input_ids,
      std::vector<std::vector<int>> &output_ids);
};

}  // namespace query_managing
}  // namespace dbmstodspi