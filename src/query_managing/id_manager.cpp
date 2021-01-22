#include "id_manager.hpp"

#include <stdexcept>

#include "operation_types.hpp"
#include <algorithm>

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
    std::vector<std::vector<int>>& input_ids,
    std::vector<std::vector<int>>& output_ids) {
  for (int current_node_index = 0; current_node_index < all_nodes.size(); current_node_index++) {
    std::vector<int> current_node_input_ids;
    std::vector<int> current_node_output_ids;
    for (int current_stream_index = 0;
         current_stream_index <
         all_nodes[current_node_index].input_data_definition_files.size();
         current_stream_index++) {
      if (all_nodes[current_node_index].previous_nodes[current_stream_index]) {
        auto previous_node_it = std::find(all_nodes.begin(), all_nodes.end(),
                  *all_nodes[current_node_index].previous_nodes[current_stream_index]);
        if (previous_node_it == all_nodes.end()) {
          throw std::runtime_error("Something went wrong!");
        }
        int previous_node_index = previous_node_it - all_nodes.begin();

        auto previous_stream_it = std::find(
            all_nodes[previous_node_index].output_data_definition_files.begin(),
            all_nodes[previous_node_index].output_data_definition_files.end(),
            all_nodes[current_node_index]
                .input_data_definition_files[current_stream_index]);
        if (previous_stream_it ==
            all_nodes[previous_node_index].output_data_definition_files.end()) {
          throw std::runtime_error("Something went wrong!");
        }
        int previous_stream_index =
            previous_stream_it -
            all_nodes[previous_node_index].output_data_definition_files.begin();

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
          all_nodes[current_node_index].output_data_definition_files.size()) {
        current_node_output_ids.push_back(
            current_node_input_ids[current_stream_index]);
      }
    }
    for (int current_stream_index =
             all_nodes[current_node_index].input_data_definition_files.size();
         current_stream_index <
         all_nodes[current_node_index].output_data_definition_files.size();
         current_stream_index++) {
      if (available_ids_.empty()) {
        throw std::runtime_error("Out of IDs!");
      }
      current_node_output_ids.push_back(available_ids_.top());
      available_ids_.pop();
    }
    input_ids.push_back(current_node_input_ids);
    output_ids.push_back(current_node_output_ids);
  }
}
