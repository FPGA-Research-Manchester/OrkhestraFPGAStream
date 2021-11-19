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
  for (const auto& potential_current_node : previous_node->next_nodes) {
    int counter = 0;
    if (potential_current_node.get() == current_node) {
      if (index != -1) {
        throw std::runtime_error(
            "Currently can't support the same module taking multiple inputs "
            "from another module!");
      }
      index = counter;
    }
  }
  if (index == -1) {
    throw std::runtime_error("No current node found!");
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
    for (const auto& [current_node_index, current_stream_index] :
         GetCurrentNodeIndexesByName(graph, next_node_name, node_name)) {
      graph.at(next_node_name).data_tables.at(current_node_index) =
          table_names.at(current_stream_index);
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
    throw std::runtime_error(
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
    for (const auto& [previous_node_name, node_index] :
         graph.at(potential_node_name).before_nodes) {
      if (std::find(past_nodes.begin(), past_nodes.end(), previous_node_name) ==
          past_nodes.end()) {
        auto search = std::find(potential_nodes.begin(), potential_nodes.end(),
                                previous_node_name);
        if (search != potential_nodes.end()) {
          potential_nodes.erase(search);
        }
      }
    }
  }
  return potential_nodes;
}

void QuerySchedulingHelper::RemoveNodeFromGraph(
    std::map<std::string, SchedulingQueryNode>& graph, std::string node_name) {
  if (graph.at(node_name).after_nodes.empty()) {
    graph.erase(node_name);
    for (const auto& [previous_node_name, stream_index] :
         graph.at(node_name).before_nodes) {
      graph.at(previous_node_name).after_nodes.at(stream_index) = "";
    }
  } else if (graph.at(node_name).before_nodes.size() > 1) {
    throw std::runtime_error("Can't remove node with multiple inputs!");
  } else {
    for (int stream_index = 0;
         stream_index < graph.at(node_name).after_nodes.size();
         stream_index++) {
      auto next_node_name = graph.at(node_name).after_nodes.at(stream_index);
      std::pair<std::string, int> current_link = {node_name, stream_index};
      auto search =
          std::find(graph.at(next_node_name).before_nodes.begin(),
                    graph.at(next_node_name).before_nodes.end(), current_link);
      if (search == graph.at(next_node_name).before_nodes.end()) {
        throw std::runtime_error("Node linking error!");
      } else {
        auto previous_node_name =
            graph.at(node_name).before_nodes.front().first;
        search->first = previous_node_name;
        search->second = graph.at(node_name).before_nodes.front().second;
        if (graph.find(previous_node_name) != graph.end()) {
          graph.at(previous_node_name)
              .after_nodes.at(graph.at(node_name).before_nodes.front().second) =
              next_node_name;
        }
      }
    }
    graph.erase(node_name);
  }
}
