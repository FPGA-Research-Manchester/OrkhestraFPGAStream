#pragma once

#include "state_interface.hpp"

using easydspi::dbmstodspi::GraphProcessingFSMInterface;

namespace easydspi::dbmstodspi {

class InitState : public StateInterface {
 public:
  ~InitState() override = default;

  std::unique_ptr<StateInterface> execute(GraphProcessingFSMInterface* fsm) override;
};
}  // namespace easydspi::dbmstodspi