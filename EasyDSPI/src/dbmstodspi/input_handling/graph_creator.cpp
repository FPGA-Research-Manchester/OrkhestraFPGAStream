#include "graph_creator.hpp"

#include "graph.hpp"

using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::dbmstodspi::Graph;
using easydspi::dbmstodspi::GraphCreator;

std::unique_ptr<ExecutionPlanGraphInterface> GraphCreator::makeGraph() {
  return std::make_unique<Graph>();
}