#pragma once

#include <memory>

#include "config.hpp"
#include "execution_plan_graph_interface.hpp"

using easydspi::core_interfaces::Config;
using easydspi::core_interfaces::ExecutionPlanGraphInterface;

namespace easydspi::dbmstodspi {

class GraphProcessingFSMInterface {
 private:
 public:
  virtual ~GraphProcessingFSMInterface() = default;

  virtual void setFinishedFlag() = 0;

  virtual std::string getCurrentGraphData() = 0;
  virtual void setCurrentGraphData(std::string new_data) = 0;
  virtual const Config& getCurrentConfig() = 0;
  virtual void setCurrentConfig(const Config& new_config) = 0;
  virtual const ExecutionPlanGraphInterface* getInitialGraph() = 0;
  virtual const Config& getInitialConfig() = 0;
};
}  // namespace easydspi::dbmstodspi