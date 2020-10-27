#include "join_setup.hpp"

// Hardcoded setup for joining customer table to the car table
void JoinSetup::SetupJoinModule(JoinInterface& join_module,
                                int first_input_stream_id,
                                int second_input_stream_id,
                                int output_stream_id) {
  join_module.DefineOutputStream(2, first_input_stream_id,
                                 second_input_stream_id, output_stream_id);
  join_module.SetFirstInputStreamChunkCount(2);
  join_module.SetSecondInputStreamChunkCount(1);

  SetupTimeMultiplexer(join_module);

  join_module.StartPrefetchingData();
}

void JoinSetup::SetupTimeMultiplexer(JoinInterface& join_module) {
  // Stream A
  // Chunk 0
  for (int i = 0; i < 16; i++) {
    join_module.SelectOutputDataElement(0, 0, i, true);
  }
  // Chunk 1
  join_module.SelectOutputDataElement(1, 1, 15, true);
  join_module.SelectOutputDataElement(1, 1, 14, true);
  // Stream B
  join_module.SelectOutputDataElement(1, 0, 13, false);
  join_module.SelectOutputDataElement(1, 0, 12, false);
  join_module.SelectOutputDataElement(1, 0, 11, false);
  join_module.SelectOutputDataElement(1, 0, 10, false);
  join_module.SelectOutputDataElement(1, 0, 9, false);
  join_module.SelectOutputDataElement(1, 0, 8, false);
  join_module.SelectOutputDataElement(1, 0, 7, false);
  join_module.SelectOutputDataElement(1, 0, 6, false);
}