#pragma once
#include <stack>
#include <vector>

#include "query_scheduling_data.hpp"
class IDManager {
 private:
  std::stack<int> available_ids_;

 public:
  IDManager();
  void MakeIDAvailable(int available_id);

  void AllocateStreamIDs(
      const std::vector<query_scheduling_data::QueryNode> node_vector,
      std::vector<std::vector<int>>& input_ids,
      std::vector<std::vector<int>>& output_ids);
};