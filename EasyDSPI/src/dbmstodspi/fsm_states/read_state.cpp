#include "read_state.hpp"

#include "execute_state.hpp"

using easydspi::dbmstodspi::ExecuteState;
using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::ReadState;
using easydspi::dbmstodspi::StateInterface;

std::unique_ptr<StateInterface> ReadState::execute(
    GraphProcessingFSMInterface* fsm) {
  // Read some data or do something

  return std::make_unique<ExecuteState>();
}