#include "id_manager.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"

using namespace dbmstodspi::query_managing;

IDManager::IDManager() {
  for (int available_index = 15; available_index >= 0; available_index--) {
    available_ids_.push(available_index);
  }
}

void IDManager::MakeIDAvailable(int available_id) {
  available_ids_.push(available_id);
}

void IDManager::AllocateStreamIDs(
    const std::vector<query_scheduling_data::QueryNode> all_nodes,
    std::vector<std::vector<int>> &input_ids,
    std::vector<std::vector<int>> &output_ids) {
  for (int current_node_index = 0; current_node_index < all_nodes.size();
       current_node_index++) {
    std::vector<int> current_node_input_ids;
    std::vector<int> current_node_output_ids;
    AllocateInputIDs(all_nodes[current_node_index], all_nodes,
                     current_node_input_ids,
                     output_ids, current_node_output_ids);
    AllocateLeftoverOutputIDs(all_nodes[current_node_index],
                              current_node_output_ids);
    input_ids.push_back(current_node_input_ids);
    output_ids.push_back(current_node_output_ids);
  }
}

void IDManager::AllocateInputIDs(
    const query_scheduling_data::QueryNode &current_node,
    const std::vector<query_scheduling_data::QueryNode> &all_nodes,
    std::vector<int> &current_node_input_ids,
    std::vector<std::vector<int>> &output_ids,
    std::vector<int> &current_node_output_ids) {
  for (int current_stream_index = 0;
       current_stream_index < current_node.input_data_definition_files.size();
       current_stream_index++) {
    if (current_node.previous_nodes[current_stream_index]) {
      int previous_node_index = FindElementIndex(
          all_nodes, *current_node.previous_nodes[current_stream_index]);

      int previous_stream_index = FindElementIndex(
          all_nodes[previous_node_index].output_data_definition_files,
          current_node.input_data_definition_files[current_stream_index]);

      current_node_input_ids.push_back(
          output_ids[previous_node_index][previous_stream_index]);
    } else {
      if (available_ids_.empty()) {
        throw std::runtime_error("Out of IDs!");
      }
      current_node_input_ids.push_back(available_ids_.top());
      available_ids_.pop();
    }
    if (current_stream_index <
        current_node.output_data_definition_files.size()) {
      current_node_output_ids.push_back(
          current_node_input_ids[current_stream_index]);
    }
  }
}

void IDManager::AllocateLeftoverOutputIDs(
    const query_scheduling_data::QueryNode &current_node,
    std::vector<int> &current_node_output_ids) {
  for (int current_stream_index =
           current_node.input_data_definition_files.size();
       current_stream_index < current_node.output_data_definition_files.size();
       current_stream_index++) {
    if (available_ids_.empty()) {
      throw std::runtime_error("Out of IDs!");
    }
    current_node_output_ids.push_back(available_ids_.top());
    available_ids_.pop();
  }
}

template <typename T>
auto IDManager::FindElementIndex(const std::vector<T> &vector, const T &element)
    -> int {
  auto vector_iterator = std::find(vector.begin(), vector.end(), element);
  if (vector_iterator == vector.end()) {
    throw std::runtime_error("Something went wrong!");
  }
  return vector_iterator - vector.begin();
}
