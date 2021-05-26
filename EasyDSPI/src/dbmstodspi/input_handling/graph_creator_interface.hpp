#pragma once

#include "execution_plan_graph_interface.hpp"
#include <memory>

using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::dbmstodspi {
class GraphCreatorInterface {
 public:
  virtual ~GraphCreatorInterface() = default;
  virtual std::unique_ptr<ExecutionPlanGraphInterface> makeGraph() = 0;
};
}  // namespace easydspi::dbmstodspi