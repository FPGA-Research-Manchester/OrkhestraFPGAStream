#include "execute_state.hpp"

using easydspi::dbmstodspi::ExecuteState;
using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::StateInterface;

std::unique_ptr<StateInterface> ExecuteState::execute(
    GraphProcessingFSMInterface* fsm) {
  // Read some data or do something
  fsm->setCurrentGraphData("Results: " + fsm->getCurrentGraphData());
  fsm->setFinishedFlag();

  return std::make_unique<ExecuteState>();
}