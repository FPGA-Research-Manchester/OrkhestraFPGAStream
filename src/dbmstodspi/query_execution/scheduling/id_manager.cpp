/*
Copyright 2022 University of Manchester

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

#include "id_manager.hpp"

#include <algorithm>
#include <stdexcept>

#include "operation_types.hpp"
#include "query_acceleration_constants.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;
using orkhestrafs::dbmstodspi::IDManager;

void IDManager::AllocateStreamIDs(
    const std::vector<QueryNode *> &all_nodes,
    std::map<std::string, std::vector<int>> &input_ids,
    std::map<std::string, std::vector<int>> &output_ids) {
  auto available_ids = SetUpAvailableIDs();

  for (const auto &current_node : all_nodes) {
    std::vector<int> current_node_input_ids;
    std::vector<int> current_node_output_ids;
    AllocateInputIDs(current_node, current_node_input_ids, output_ids,
                     current_node_output_ids, available_ids);
    AllocateLeftoverOutputIDs(current_node, current_node_output_ids,
                              available_ids);
    input_ids.insert({current_node->node_name, current_node_input_ids});
    output_ids.insert({current_node->node_name, current_node_output_ids});
  }
}

auto IDManager::SetUpAvailableIDs() -> std::stack<int> {
  std::stack<int> available_ids;
  for (int available_index =
           query_acceleration_constants::kMaxIOStreamCount - 1;
       available_index >= 0; available_index--) {
    available_ids.push(available_index);
  }
  return available_ids;
}

void IDManager::AllocateInputIDs(
    const QueryNode *current_node, std::vector<int> &current_node_input_ids,
    std::map<std::string, std::vector<int>> &output_ids,
    std::vector<int> &current_node_output_ids, std::stack<int> &available_ids) {
  // This check is incorrect - Merge sort needs 1 stream but can have multiple
  // file inputs
  /*if (current_node->given_input_data_definition_files.size() <
      current_node->module_run_data.front()
          .input_data_definition_files.size()) {
    throw std::runtime_error("Incorrect number of inputs given for run!");
  }*/
  for (int current_stream_index = 0;
       current_stream_index <
       current_node->given_input_data_definition_files.size();
       current_stream_index++) {
    if (current_node->module_run_data.front()
            .input_data_definition_files.at(current_stream_index)
            .empty()) {
      const auto &previous_node =
          current_node->previous_nodes[current_stream_index];
      if (!previous_node) {
        throw std::runtime_error(
            "Previous node not found when looking for ID!");
      }
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
        current_node->given_output_data_definition_files.size()) {
      current_node_output_ids.push_back(
          current_node_input_ids[current_stream_index]);
    }
  }
}

void IDManager::AllocateLeftoverOutputIDs(
    const QueryNode *current_node, std::vector<int> &current_node_output_ids,
    std::stack<int> &available_ids) {
  for (int current_stream_index =
           current_node->given_input_data_definition_files.size();
       current_stream_index <
       current_node->given_output_data_definition_files.size();
       current_stream_index++) {
    if (available_ids.empty()) {
      throw std::runtime_error("Out of IDs!");
    }
    current_node_output_ids.push_back(available_ids.top());
    available_ids.pop();
  }
}

auto IDManager::FindStreamIndex(const std::vector<QueryNode *> &stream_vector,
                                const QueryNode *node) -> int {
  for (int i = 0; i < stream_vector.size(); i++) {
    if (stream_vector[i] && stream_vector[i] == node) {
      return i;
    }
  }
  throw std::runtime_error("Something went wrong!");
}
