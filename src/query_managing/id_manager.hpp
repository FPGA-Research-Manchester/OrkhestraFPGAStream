#pragma once
#include <stack>
#include <vector>

#include "query_scheduling_data.hpp"
class IDManager {
 private:
  std::stack<int> available_ids_;

 public:
  IDManager();
  void FindAvailableIDs(const query_scheduling_data::QueryNode& query_node,
                        std::vector<int>& input_stream_id_vector,
                        std::vector<int>& output_stream_id_vector);

  void MakeIDAvailable(int available_id);
};