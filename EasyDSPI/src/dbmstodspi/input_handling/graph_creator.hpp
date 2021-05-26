#pragma once

#include "graph_creator_interface.hpp"

using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::dbmstodspi {
class GraphCreator : public GraphCreatorInterface {
 public:
  ~GraphCreator() override = default;
  std::unique_ptr<ExecutionPlanGraphInterface> makeGraph() override;
};
}  // namespace easydspi::dbmstodspi