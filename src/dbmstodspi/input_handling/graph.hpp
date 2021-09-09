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

#include "execution_plan_graph_interface.hpp"
#include "query_scheduling_data.hpp"

using orkhestrafs::core_interfaces::ExecutionPlanGraphInterface;
using orkhestrafs::core_interfaces::query_scheduling_data::QueryNode;

namespace orkhestrafs::dbmstodspi {
class Graph : public ExecutionPlanGraphInterface {
 private:
  std::vector<std::shared_ptr<QueryNode>> root_nodes_;

 public:
  ~Graph() override = default;

  explicit Graph(std::vector<std::shared_ptr<QueryNode>> graph_data)
      : root_nodes_{std::move(graph_data)} {}

  auto ExportRootNodes() -> std::vector<std::shared_ptr<QueryNode>> override;
  auto IsEmpty() -> bool override;
};
}  // namespace orkhestrafs::dbmstodspi