/*
Copyright 2021 University of Manchester

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

#include "query_scheduling_helper.hpp"

#include <algorithm>
#include <stdexcept>

using orkhestrafs::dbmstodspi::QuerySchedulingHelper;

auto QuerySchedulingHelper::FindNodePtrIndex(QueryNode* current_node,
                                             QueryNode* previous_node) -> int {
  int index = -1;
  int counter = 0;
  for (const auto& potential_current_node : previous_node->next_nodes) {
    if (potential_current_node != nullptr &&
        potential_current_node.get() == current_node) {
      if (index != -1) {
        throw std::logic_error(
            "Currently can't support the same module taking multiple inputs "
            "from another module!");
      }
      index = counter;
    }
    counter++;
  }
  if (index == -1) {
    throw std::logic_error("No current node found!");
  }
  return index;
}

// TODO: Check that it is sorted by the desired column.
auto QuerySchedulingHelper::IsTableSorted(TableMetadata table_data) -> bool {
  return table_data.sorted_status.size() == 1 &&
         table_data.sorted_status.at(0).start_position == 0 &&
         table_data.sorted_status.at(0).length == table_data.record_count;
}

void QuerySchedulingHelper::AddNewTableToNextNodes(
    std::map<std::string, SchedulingQueryNode>& graph, std::string node_name,
    const std::vector<std::string>& table_names) {
  for (const auto& next_node_name : graph.at(node_name).after_nodes) {
    if (!next_node_name.empty()) {
      for (const auto& [current_node_index, current_stream_index] :
           GetCurrentNodeIndexesByName(graph, next_node_name, node_name)) {
        graph.at(next_node_name).data_tables.at(current_node_index) =
            table_names.at(current_stream_index);
      }
    }
  }
}

auto QuerySchedulingHelper::GetCurrentNodeIndexesByName(
    const std::map<std::string, SchedulingQueryNode>& graph,
    std::string next_node_name, std::string current_node_name)
    -> std::vector<std::pair<int, int>> {
  std::vector<std::pair<int, int>> resulting_indexes;
  for (int potential_current_node_index = 0;
       potential_current_node_index <
       graph.at(next_node_name).before_nodes.size();
       potential_current_node_index++) {
    if (graph.at(next_node_name)
            .before_nodes.at(potential_current_node_index)
            .first == current_node_name) {
      auto stream_index = graph.at(next_node_name)
                              .before_nodes.at(potential_current_node_index)
                              .second;
      resulting_indexes.push_back({potential_current_node_index, stream_index});
    }
  }
  if (resulting_indexes.empty()) {
    throw std::logic_error(
        "No next nodes found with the expected dependency");
  }
  return resulting_indexes;
}

auto QuerySchedulingHelper::GetNewAvailableNodesAfterSchedulingGivenNode(
    std::string node_name, const std::vector<std::string>& past_nodes,
    const std::map<std::string, SchedulingQueryNode>& graph)
    -> std::vector<std::string> {
  std::vector<std::string> potential_nodes = graph.at(node_name).after_nodes;
  for (const auto& potential_node_name : graph.at(node_name).after_nodes) {
    if (!potential_node_name.empty()) {
      for (const auto& [previous_node_name, node_index] :
           graph.at(potential_node_name).before_nodes) {
        if (!previous_node_name.empty() &&
            std::find(past_nodes.begin(), past_nodes.end(),
                      previous_node_name) == past_nodes.end()) {
          auto search = std::find(potential_nodes.begin(),
                                  potential_nodes.end(), previous_node_name);
          if (search != potential_nodes.end()) {
            potential_nodes.erase(search);
          }
        }
      }
    }
  }
  potential_nodes.erase(
      std::remove(potential_nodes.begin(), potential_nodes.end(), ""),
      potential_nodes.end());
  return potential_nodes;
}

// TODO: This needs improving to support multi inputs and outputs
void QuerySchedulingHelper::RemoveNodeFromGraph(
    std::map<std::string, SchedulingQueryNode>& graph, std::string node_name) {

  // We just support one in and one out currently.
  if (graph.at(node_name).before_nodes.size() != 1 ||
      graph.at(node_name).after_nodes.size() != 1) {
    throw std::logic_error(
        "Only nodes with one input and one output are supported!");
  }

  auto before_node = graph.at(node_name).before_nodes.front();
  auto after_node = graph.at(node_name).after_nodes.front();
  // TODO: No need for the temp copies.
  if (after_node.empty() && before_node.second == -1) {
    // Do nothing
  } else if (after_node.empty()) {
    if (graph.find(before_node.first) != graph.end()) {
      auto new_after_nodes = graph.at(before_node.first).after_nodes;
      new_after_nodes.at(before_node.second) = after_node;
      graph.at(before_node.first).after_nodes = new_after_nodes;
    }
  } else if (before_node.second == -1) {
    int index =
        GetCurrentNodeIndexesByName(graph, after_node, node_name).front().first;
    auto new_before_nodes = graph.at(after_node).before_nodes;
    new_before_nodes.at(index) = before_node;
    graph.at(after_node).before_nodes = new_before_nodes;
  } else {
    if (graph.find(before_node.first) != graph.end()) {
      auto new_after_nodes = graph.at(before_node.first).after_nodes;
      new_after_nodes.at(before_node.second) = after_node;
      graph.at(before_node.first).after_nodes = new_after_nodes;
    }
    int index =
        GetCurrentNodeIndexesByName(graph, after_node, node_name).front().first;
    auto new_before_nodes = graph.at(after_node).before_nodes;
    new_before_nodes.at(index) = before_node;
    graph.at(after_node).before_nodes = new_before_nodes;
  }

  graph.erase(node_name);
}
