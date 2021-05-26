﻿#pragma once

#include <memory>

#include "fsm_runner_interface.hpp"

using easydspi::dbmstodspi::GraphProcessingFSMInterface;

namespace easydspi::dbmstodspi {

class StateInterface {
 public:
  virtual ~StateInterface() = default;

  virtual std::unique_ptr<StateInterface> execute(GraphProcessingFSMInterface* fsm) = 0;
};
}  // namespace easydspi::dbmstodspi