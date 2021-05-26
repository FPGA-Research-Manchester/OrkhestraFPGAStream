#pragma once

#include "state_interface.hpp"

using easydspi::dbmstodspi::GraphProcessingFSMInterface;

namespace easydspi::dbmstodspi {

class ReadState : public StateInterface {
 public:
  ~ReadState() override = default;

  std::unique_ptr<StateInterface> execute(GraphProcessingFSMInterface* fsm) override;
};
}  // namespace easydspi::dbmstodspi