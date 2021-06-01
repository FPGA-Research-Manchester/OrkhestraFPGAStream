#include "execution_manager.hpp"

using easydspi::core::core_execution::ExecutionManager;

void ExecutionManager::setFinishedFlag() { busy_flag_ = false; }

std::vector<std::string> ExecutionManager::execute(
    std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
        execution_input) {
  initial_config_ = execution_input.second;
  initial_graph_ = std::move(execution_input.first);

  busy_flag_ = true;
  while (busy_flag_) {
    auto new_state = current_state_->execute(this);
    if (new_state) {
      current_state_ = std::move(new_state);
    }
  }
  return {getCurrentGraphData()};
}

std::string ExecutionManager::getCurrentGraphData() {
  return current_graph_data_;
}
void ExecutionManager::setCurrentGraphData(std::string new_data) {
  current_graph_data_ = new_data;
}
const Config& ExecutionManager::getCurrentConfig() { return current_config_; }
void ExecutionManager::setCurrentConfig(const Config& new_config) {
  current_config_ = new_config;
}
const ExecutionPlanGraphInterface* ExecutionManager::getInitialGraph() {
  return initial_graph_.get();
}
const Config& ExecutionManager::getInitialConfig() { return initial_config_; }