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
#include "graph.hpp"

#include <algorithm>

using orkhestrafs::dbmstodspi::Graph;

auto Graph::ExportRootNodes() -> std::vector<std::shared_ptr<QueryNode>> {
  return std::move(root_nodes_);
}

auto Graph::IsEmpty() -> bool { return root_nodes_.empty(); }

auto Graph::GetRootNodesPtrs() -> std::vector<QueryNode*> {
  std::vector<QueryNode*> node_ptrs;
  for (const auto& node : root_nodes_) {
    node_ptrs.push_back(node.get());
  }
  return node_ptrs;
}

auto Graph::GetAllNodesPtrs() -> std::vector<QueryNode*> {
  std::vector<QueryNode*> all_nodes_vector;
  for (const auto& node : root_nodes_) {
    all_nodes_vector.push_back(node.get());
    for (const auto& next_node : node->next_nodes) {
      AddNextNodeToVector(next_node.get(), all_nodes_vector);
    }
  }
  return all_nodes_vector;
}

void Graph::AddNextNodeToVector(QueryNode* current_node,
                                std::vector<QueryNode*>& all_nodes_vector) {
  if (std::find(all_nodes_vector.begin(), all_nodes_vector.end(),
                current_node) == all_nodes_vector.end()) {
    all_nodes_vector.push_back(current_node);
    for (const auto& next_node : current_node->next_nodes) {
      if (next_node) {
        AddNextNodeToVector(next_node.get(), all_nodes_vector);
      }
    }
  }
}