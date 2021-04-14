#include "multiplication_setup.hpp"

#include <array>

using namespace dbmstodspi::fpga_managing;

void MultiplicationSetup::SetupMultiplicationModule(
    modules::MultiplicationInterface& multiplication_module,
    int active_stream_id) {
  std::array<bool, 16> active_streams;
  active_streams.fill(false);
  active_streams.at(active_stream_id) = true;
  multiplication_module.DefineActiveStreams(active_streams);

  std::array<bool, 8> selected_positions;
  selected_positions.fill(false);
  selected_positions.at(7) = true;
  multiplication_module.ChooseMultiplicationResults(0, selected_positions);
}