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

#include "run_linker.hpp"

#include <algorithm>
#include <stdexcept>

using orkhestrafs::dbmstodspi::RunLinker;

auto RunLinker::LinkPeripheralNodesFromGivenRuns(
    std::queue<std::pair<std::vector<ScheduledModule>,
                         std::vector<std::shared_ptr<QueryNode>>>>
        query_node_runs_queue,
    std::map<std::string,
             std::map<int, std::vector<std::pair<std::string, int>>>>&
        linked_nodes)
    -> std::queue<std::pair<std::vector<ScheduledModule>,
                            std::vector<std::shared_ptr<QueryNode>>>> {
  std::queue<std::pair<std::vector<ScheduledModule>,
                       std::vector<std::shared_ptr<QueryNode>>>>
      linked_nodes_queue;
  while (!query_node_runs_queue.empty()) {
    auto [current_set, current_query_nodes] = query_node_runs_queue.front();
    query_node_runs_queue.pop();

    CheckExternalLinks(current_query_nodes, linked_nodes);

    linked_nodes_queue.push({current_set, current_query_nodes});
  }
  return linked_nodes_queue;
}

// Method to remove next or previous nodes from a node once it has been
// scheduled
void RunLinker::CheckExternalLinks(
    const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes,
    std::map<std::string,
             std::map<int, std::vector<std::pair<std::string, int>>>>&
        linked_nodes) {
  for (const auto& node : current_query_nodes) {
    std::map<int, std::vector<std::pair<std::string, int>>> target_maps;
    for (int next_node_index = 0; next_node_index < node->next_nodes.size();
         next_node_index++) {
      std::vector<std::pair<std::string, int>> targets;
      if (!node->next_nodes[next_node_index]) {
        if (node->output_data_definition_files[next_node_index].empty()) {
          node->output_data_definition_files[next_node_index] =
              node->node_name + "_" + std::to_string(next_node_index) + ".csv";
        }
      } else if (IsNodeMissingFromTheVector(node->next_nodes[next_node_index],
                                            current_query_nodes)) {
        int current_node_location = FindPreviousNodeLocation(
            node->next_nodes[next_node_index]->previous_nodes, node);
        auto current_filename =
            node->output_data_definition_files[next_node_index];
        if (current_filename.empty() &&
            ReuseMemory(*node, *node->next_nodes[next_node_index])) {
          targets.emplace_back(node->next_nodes[next_node_index]->node_name,
                               current_node_location);
        } else {
          if (current_filename.empty()) {
            current_filename = node->node_name + "_" +
                               std::to_string(next_node_index) + ".csv";
            node->output_data_definition_files[next_node_index] =
                current_filename;
          }
          node->next_nodes[next_node_index]
              ->input_data_definition_files[current_node_location] =
              current_filename;
          node->next_nodes[next_node_index] = nullptr;
        }
      }
      if (!targets.empty()) {
        target_maps.insert({next_node_index, targets});
      }
    }
    for (auto& previous_node : node->previous_nodes) {
      auto observed_node = previous_node.lock();
      if (IsNodeMissingFromTheVector(observed_node, current_query_nodes)) {
        previous_node = std::weak_ptr<QueryNode>();
      }
    }
    if (!target_maps.empty()) {
      linked_nodes.insert({node->node_name, target_maps});
    }
  }
}

auto RunLinker::ReuseMemory(const QueryNode& /*source_node*/,
                            const QueryNode& /*target_node*/) -> bool {
  // TODO(Kaspar): needs to consider the memory capabilities
  return true;
}

auto RunLinker::IsNodeMissingFromTheVector(
    const std::shared_ptr<QueryNode>& linked_node,
    const std::vector<std::shared_ptr<QueryNode>>& current_query_nodes)
    -> bool {
  return linked_node &&
         std::find(current_query_nodes.begin(), current_query_nodes.end(),
                   linked_node) == current_query_nodes.end();
}

auto RunLinker::FindPreviousNodeLocation(
    const std::vector<std::weak_ptr<QueryNode>>& previous_nodes,
    const std::shared_ptr<QueryNode>& previous_node) -> int {
  for (int previous_node_index = 0; previous_node_index < previous_nodes.size();
       previous_node_index++) {
    auto observed_node = previous_nodes[previous_node_index].lock();
    if (observed_node == previous_node) {
      return previous_node_index;
    }
  }
  throw std::runtime_error("No node found!");
}
