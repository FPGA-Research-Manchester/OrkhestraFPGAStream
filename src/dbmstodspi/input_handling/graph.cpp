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
#include "graph.hpp"

#include <algorithm>
#include <stdexcept>

using orkhestrafs::dbmstodspi::Graph;

// Have to find nodes that have been skipped but the parent and children are
// not yet executed. If input is not null and is not in the processed nodes ->
// Throw an error if output is in the processed nodes. Just put the input to
// the output If output is not in the processed nodes -> Set to nullptr, if
// input table is "" throw error.
void Graph::DeleteNodes(
    const std::unordered_set<std::string>& deleted_node_names) {
  for (const auto& node_ptr : GetAllNodesPtrs()) {
    if (deleted_node_names.find(node_ptr->node_name) !=
        deleted_node_names.end()) {
      for (const auto input_ptr : node_ptr->previous_nodes) {
        if (input_ptr != nullptr &&
            deleted_node_names.find(input_ptr->node_name) !=
                deleted_node_names.end()) {
          if (node_ptr->previous_nodes.size() != 1) {
            throw std::runtime_error("Can't skip nodes with multiple inputs!");
          }
          for (const auto output_ptr : node_ptr->next_nodes) {
            if (output_ptr != nullptr) {
              if (deleted_node_names.find(output_ptr->node_name) !=
                  deleted_node_names.end()) {
                throw std::runtime_error(
                    "Not supporting multiple skipped nodes!");
              } else {
                FindCurrentNodeAndSetToNull(node_ptr, output_ptr);
              }
            }
          }
        }
      }
      for (const auto output_ptr : node_ptr->next_nodes) {
        if (output_ptr != nullptr &&
            deleted_node_names.find(output_ptr->node_name) ==
                deleted_node_names.end()) {
          FindCurrentNodeAndSetToNull(node_ptr, output_ptr);
        }
      }
    }
  }
  for (const auto& node_ptr : GetAllNodesPtrs()) {
    if (deleted_node_names.find(node_ptr->node_name) !=
        deleted_node_names.end()) {
      DeleteNode(node_ptr);
    }
  }
}

void Graph::FindCurrentNodeAndSetToNull(const QueryNode* node_ptr,
                                        QueryNode* output_ptr) const {
  bool found = false;
  for (int stream_id = 0; stream_id < output_ptr->previous_nodes.size();
       stream_id++) {
    if (output_ptr->previous_nodes.at(stream_id) == node_ptr) {
      if (output_ptr->given_input_data_definition_files.at(stream_id).empty()) {
        throw std::runtime_error("Table not defined when deleting nodes!");
      } else {
        output_ptr->previous_nodes.at(stream_id) = nullptr;
        found = true;
      }
    }
  }
  if (!found) {
    throw std::runtime_error("Output node not linked to current skipped node!");
  }
}

void Graph::DeleteNode(QueryNode* deleted_node) {
  all_nodes_.erase(
      std::remove(all_nodes_.begin(), all_nodes_.end(), *deleted_node),
      all_nodes_.end());
}

auto Graph::IsEmpty() -> bool { return all_nodes_.empty(); }

auto Graph::GetRootNodesPtrs() -> std::vector<QueryNode*> {
  std::vector<QueryNode*> node_ptrs;
  for (auto& node : all_nodes_) {
    if (node.previous_nodes.empty()) {
      throw std::runtime_error("Currently we don't support 0 input nodes");
    }
    if (std::all_of(node.previous_nodes.begin(), node.previous_nodes.end(),
                    [](const auto& ptr) { return ptr == nullptr; })) {
      node_ptrs.push_back(&node);
    }
  }
  return std::move(node_ptrs);
}

auto Graph::GetAllNodesPtrs() -> std::vector<QueryNode*> {
  std::vector<QueryNode*> node_ptrs(all_nodes_.size());
  for (auto& node : all_nodes_) {
    node_ptrs.push_back(&node);
  }
  return std::move(node_ptrs);
}