#include "init_state.hpp"

#include "read_state.hpp"

using easydspi::dbmstodspi::GraphProcessingFSMInterface;
using easydspi::dbmstodspi::InitState;
using easydspi::dbmstodspi::ReadState;
using easydspi::dbmstodspi::StateInterface;

std::unique_ptr<StateInterface> InitState::execute(
    GraphProcessingFSMInterface* fsm) {
  fsm->setCurrentGraphData(fsm->getInitialGraph()->exportData());
  fsm->setCurrentConfig(fsm->getInitialConfig());

  return std::make_unique<ReadState>();
}