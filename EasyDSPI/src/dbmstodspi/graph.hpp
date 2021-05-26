#pragma once

#include "execution_plan_graph_interface.hpp"

using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::dbmstodspi {
class Graph : public ExecutionPlanGraphInterface {
 private:
  std::string stored_data_;

 public:
  ~Graph() override = default;
  void insertData(std::string given_data) override;
  std::string exportData() const override;
};
}  // namespace easydspi::dbmstodspi