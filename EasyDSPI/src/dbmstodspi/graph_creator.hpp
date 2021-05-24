#pragma once

#include "execution_plan_graph_interface.hpp"
#include <memory>

using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::dbmstodspi {
class GraphCreator {
 public:
  std::unique_ptr<ExecutionPlanGraphInterface> makeGraph();
};
}  // namespace easydspi::dbmstodspi