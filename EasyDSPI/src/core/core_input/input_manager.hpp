#pragma once

#include "input_manager_interface.hpp"
#include "graph_creator.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;
using easydspi::core_interfaces::InputManagerInterface;
using easydspi::dbmstodspi::GraphCreator;

namespace easydspi::core::core_input {

class InputManager : public InputManagerInterface {
 private:
  GraphCreator graph_creator_;

 public:
  ~InputManager() override = default;
  InputManager(const GraphCreator &graph_creator)
      : graph_creator_{graph_creator} {};

  std::pair<std::unique_ptr<ExecutionPlanGraphInterface>, Config> parse(
      std::string input_filename, std::string config_filename) override;
};
}  // namespace easydspi::core::core_input