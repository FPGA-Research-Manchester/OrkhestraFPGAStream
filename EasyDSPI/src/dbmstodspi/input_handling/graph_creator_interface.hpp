#pragma once

#include <memory>
#include <string>

#include "execution_plan_graph_interface.hpp"

using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::dbmstodspi {
class GraphCreatorInterface {
 public:
  virtual ~GraphCreatorInterface() = default;
  virtual std::unique_ptr<ExecutionPlanGraphInterface> makeGraph(
      std::string graph_def_filename) = 0;
};
}  // namespace easydspi::dbmstodspi