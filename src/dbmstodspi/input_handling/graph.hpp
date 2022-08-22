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
#pragma once

#include <memory>

#include "execution_plan_graph_interface.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::ExecutionPlanGraphInterface;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

namespace orkhestrafs::dbmstodspi {
class Graph : public ExecutionPlanGraphInterface {
 private:
  std::vector<std::unique_ptr<QueryNode>> all_nodes_;
  void DeleteNode(QueryNode* deleted_node);
  static void FindCurrentNodeAndSetToNull(const QueryNode* node_ptr,
                                          QueryNode* output_ptr);

 public:
  ~Graph() override = default;

  explicit Graph(std::vector<std::unique_ptr<QueryNode>> graph_data)
      : all_nodes_{std::move(graph_data)} {}

  void DeleteNodes(
      const std::unordered_set<std::string>& deleted_node_names) override;
  auto IsEmpty() -> bool override;
  auto GetRootNodesPtrs() -> std::vector<QueryNode*> override;
  auto GetAllNodesPtrs() -> std::vector<QueryNode*> override;
  void ImportNodes(std::vector<std::unique_ptr<QueryNode>> new_nodes) override;
};
}  // namespace orkhestrafs::dbmstodspi