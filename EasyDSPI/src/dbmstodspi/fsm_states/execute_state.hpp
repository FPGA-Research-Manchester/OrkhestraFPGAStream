#pragma once

#include "state_interface.hpp"

using easydspi::dbmstodspi::GraphProcessingFSMInterface;

namespace easydspi::dbmstodspi {

class ExecuteState : public StateInterface {
 public:
  ~ExecuteState() override = default;

  std::unique_ptr<StateInterface> execute(GraphProcessingFSMInterface* fsm) override;
};
}  // namespace easydspi::dbmstodspi