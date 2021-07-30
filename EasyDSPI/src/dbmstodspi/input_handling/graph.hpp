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
#include "execution_plan_node.hpp"

using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::core_interfaces::ExecutionPlanNode;

namespace easydspi::dbmstodspi {
class Graph : public ExecutionPlanGraphInterface {
 private:
  std::vector<std::shared_ptr<ExecutionPlanNode>> stored_data_;

 public:
  ~Graph() override = default;

  Graph(std::vector<std::shared_ptr<ExecutionPlanNode>> graph_data)
      : stored_data_{graph_data} {}

  void insertData(std::string given_data) override;
  std::string exportData() const override;
};
}  // namespace easydspi::dbmstodspi