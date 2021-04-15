#include "multiplication_setup.hpp"

#include <array>

using namespace dbmstodspi::fpga_managing;

void MultiplicationSetup::SetupMultiplicationModule(
    modules::MultiplicationInterface& multiplication_module,
    std::vector<int> active_stream_ids,
    const std::vector<std::vector<int>>& operation_parameters) {
  std::array<bool, 16> active_streams;
  active_streams.fill(false);
  for (const auto& active_stream_id : active_stream_ids) {
    active_streams.at(active_stream_id) = true;
  }
  multiplication_module.DefineActiveStreams(active_streams);

  for
    (const auto& multiplication_selections : operation_parameters) {
    std::array<bool, 8> selected_positions;
    for (int position = 0; position < 8; position++) {
      selected_positions.at(position) =
          multiplication_selections.at(8 - position);
    }
    multiplication_module.ChooseMultiplicationResults(multiplication_selections.at(0),
                                                      selected_positions);
  }
}