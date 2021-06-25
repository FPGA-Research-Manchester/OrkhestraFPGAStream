#include "id_manager.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"
#include "util.hpp"

using namespace dbmstodspi::query_managing;

using dbmstodspi::util::FindPositionInVector;

void IDManager::AllocateStreamIDs(
    const std::vector<query_scheduling_data::QueryNode> &all_nodes,
    std::map<std::string, std::vector<int>> &input_ids,
    std::map<std::string, std::vector<int>> &output_ids) {
  auto available_ids = SetUpAvailableIDs();

  for (const auto &current_node : all_nodes) {
    std::vector<int> current_node_input_ids;
    std::vector<int> current_node_output_ids;
    AllocateInputIDs(current_node, current_node_input_ids,
                     output_ids, current_node_output_ids, available_ids);
    AllocateLeftoverOutputIDs(current_node, current_node_output_ids,
                              available_ids);
    input_ids.insert({current_node.node_name, current_node_input_ids});
    output_ids.insert({current_node.node_name, current_node_output_ids});
  }
}

auto IDManager::SetUpAvailableIDs() -> std::stack<int> {
  std::stack<int> available_ids;
  for (int available_index =
           fpga_managing::query_acceleration_constants::kMaxIOStreamCount - 1;
       available_index >= 0; available_index--) {
    available_ids.push(available_index);
  }
  return available_ids;
}

void IDManager::AllocateInputIDs(
    const query_scheduling_data::QueryNode &current_node,
    std::vector<int> &current_node_input_ids,
    std::map<std::string, std::vector<int>> &output_ids,
    std::vector<int> &current_node_output_ids, std::stack<int>& available_ids) {
  for (int current_stream_index = 0;
       current_stream_index < current_node.input_data_definition_files.size();
       current_stream_index++) {
    auto previous_node =
        current_node.previous_nodes[current_stream_index].lock();
    if (previous_node) {
      int previous_stream_index =
          FindStreamIndex(previous_node->next_nodes, current_node);

      current_node_input_ids.push_back(
          output_ids[previous_node->node_name][previous_stream_index]);
    } else {
      if (available_ids.empty()) {
        throw std::runtime_error("Out of IDs!");
      }
      current_node_input_ids.push_back(available_ids.top());
      available_ids.pop();
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
    std::vector<int> &current_node_output_ids, std::stack<int>& available_ids) {
  for (int current_stream_index =
           current_node.input_data_definition_files.size();
       current_stream_index < current_node.output_data_definition_files.size();
       current_stream_index++) {
    if (available_ids.empty()) {
      throw std::runtime_error("Out of IDs!");
    }
    current_node_output_ids.push_back(available_ids.top());
    available_ids.pop();
  }
}

auto IDManager::FindStreamIndex(
    const std::vector<std::shared_ptr<query_scheduling_data::QueryNode>>
        &stream_vector,
    const query_scheduling_data::QueryNode &node) -> int {
  for (int i = 0; i < stream_vector.size(); i++) {
    if (*stream_vector[i] == node) {
      return i;
    }
  }
  throw std::runtime_error("Something went wrong!");
}
