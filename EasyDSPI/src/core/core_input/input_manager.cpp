#include "input_manager.hpp"

#include <memory>

using easydspi::core::core_input::InputManager;

std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
InputManager::parse(std::string input_filename, std::string config_filename) {
  Config config = {};
  config.driver_library.insert({"1", config_filename});

  auto graph = graph_creator_.makeGraph();
  graph->insertData(input_filename);

  return std::make_pair(std::move(graph), config);
}