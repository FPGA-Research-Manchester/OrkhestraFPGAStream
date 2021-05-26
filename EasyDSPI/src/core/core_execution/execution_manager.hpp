#pragma once

#include <string>

#include "execution_manager_interface.hpp"
#include "fsm_runner_interface.hpp"
#include "state_interface.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionManagerInterface;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::StateInterface;

namespace easydspi::core::core_execution {

class ExecutionManager : public ExecutionManagerInterface,
                         public GraphProcessingFSMInterface {
 private:
  std::unique_ptr<ExecutionPlanGraphInterface> initial_graph_;
  std::string current_graph_data_;
  Config initial_config_;
  Config current_config_;

  std::unique_ptr<StateInterface> current_state_;
  bool busy_flag_ = false;

 public:
  ~ExecutionManager() override = default;
  ExecutionManager(std::unique_ptr<StateInterface> start_state)
      : current_state_{std::move(start_state)} {}

  void setFinishedFlag() override;

  std::vector<std::string> execute(
      std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config>
          execution_input) override;
  std::string getCurrentGraphData() override;
  void setCurrentGraphData(std::string new_data) override;
  const Config& getCurrentConfig() override;
  void setCurrentConfig(const Config& new_config) override;
  const ExecutionPlanGraphInterface* getInitialGraph() override;
  const Config& getInitialConfig() override;
};
}  // namespace easydspi::core::core_execution