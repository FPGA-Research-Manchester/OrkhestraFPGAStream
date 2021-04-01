#pragma once
#include <stack>
#include <vector>

#include "query_scheduling_data.hpp"
class IDManager {
 private:
  std::stack<int> available_ids_{};
  template <typename T>
  auto FindElementIndex(const std::vector<T> &vector, const T &element) -> int;

  void AllocateInputIDs(
      const query_scheduling_data::QueryNode &current_node,
      const std::vector<query_scheduling_data::QueryNode> &all_nodes,
      std::vector<int> &current_node_input_ids,
      std::vector<std::vector<int>> &output_ids,
      std::vector<int> &current_node_output_ids);
  void AllocateLeftoverOutputIDs(const query_scheduling_data::QueryNode &current_node,
                                 std::vector<int> &current_node_output_ids);

 public:
  IDManager();
  void MakeIDAvailable(int available_id);
  void AllocateStreamIDs(
      std::vector<query_scheduling_data::QueryNode> node_vector,
      std::vector<std::vector<int>> &input_ids,
      std::vector<std::vector<int>> &output_ids);
};