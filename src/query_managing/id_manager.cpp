#include "id_manager.hpp"

#include <stdexcept>

#include "operation_types.hpp"

IDManager::IDManager() {
  for (int available_index = 15; available_index >= 0; available_index--) {
    available_ids_.push(available_index);
  }
}

void IDManager::FindAvailableIDs(
    const query_scheduling_data::QueryNode& query_node,
    std::vector<int>& input_stream_id_vector,
    std::vector<int>& output_stream_id_vector) {
  switch (query_node.operation_type) {
    case operation_types::QueryOperation::kFilter: {
      if (query_node.output_data_definition_files.size() == 2) {
        if (available_ids_.size() < 2) {
          throw std::runtime_error("Out of IDs!");
        }
        output_stream_id_vector.push_back(available_ids_.top());
        input_stream_id_vector.push_back(available_ids_.top());
        available_ids_.pop();
        output_stream_id_vector.push_back(available_ids_.top());
        available_ids_.pop();
      } else {
        output_stream_id_vector.push_back(available_ids_.top());
        input_stream_id_vector.push_back(available_ids_.top());
        available_ids_.pop();
      }
      break;
    }
    case operation_types::QueryOperation::kJoin: {
      if (available_ids_.size() < 2) {
        throw std::runtime_error("Out of IDs!");
      }
      output_stream_id_vector.push_back(available_ids_.top());
      input_stream_id_vector.push_back(available_ids_.top());
      available_ids_.pop();
      input_stream_id_vector.push_back(available_ids_.top());
      available_ids_.pop();
      break;
    }
    case operation_types::QueryOperation::kMergeSort:
    case operation_types::QueryOperation::kPassThrough:
    case operation_types::QueryOperation::kLinearSort: {
      if (available_ids_.empty()) {
        throw std::runtime_error("Out of IDs!");
      }
      output_stream_id_vector.push_back(available_ids_.top());
      input_stream_id_vector.push_back(available_ids_.top());
      available_ids_.pop();
      break;
    }
  }
}

void IDManager::MakeIDAvailable(int available_id) {
  available_ids_.push(available_id);
}
