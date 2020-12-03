#include "join_setup.hpp"

#include "query_acceleration_constants.hpp"

// Hardcoded setup for joining customer table to the car table
void JoinSetup::SetupJoinModule(JoinInterface& join_module,
                                int first_input_stream_id,
                                int second_input_stream_id,
                                int output_stream_id) {
  join_module.Reset();
  join_module.DefineOutputStream(2, first_input_stream_id,
                                 second_input_stream_id, output_stream_id);
  join_module.SetFirstInputStreamChunkCount(2);
  join_module.SetSecondInputStreamChunkCount(1);

  SetupTimeMultiplexer(join_module);

  join_module.StartPrefetchingData();
}

void JoinSetup::SetupTimeMultiplexer(JoinInterface& join_module) {
  // TODO(Kaspar): Simple initial algo should be to get the chunk count for both
  // streams and then just configure first stream to go first and then second.
  // Therefore stream configuration data is needed. Stream A Chunk 0
  for (int i = 0; i < query_acceleration_constants::kDatapathWidth; i++) {
    join_module.SelectOutputDataElement(0, 0, i, false);
  }
  // Chunk 1
  join_module.SelectOutputDataElement(1, 1, 15, false);
  join_module.SelectOutputDataElement(1, 1, 14, false);
  // Stream B
  join_module.SelectOutputDataElement(1, 0, 13, true);
  join_module.SelectOutputDataElement(1, 0, 12, true);
  join_module.SelectOutputDataElement(1, 0, 11, true);
  join_module.SelectOutputDataElement(1, 0, 10, true);
  join_module.SelectOutputDataElement(1, 0, 9, true);
  join_module.SelectOutputDataElement(1, 0, 8, true);
  join_module.SelectOutputDataElement(1, 0, 7, true);
  join_module.SelectOutputDataElement(1, 0, 6, true);
}