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