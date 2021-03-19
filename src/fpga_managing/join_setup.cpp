#include "join_setup.hpp"

#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

// Hardcoded setup for joining customer table to the car table
void JoinSetup::SetupJoinModule(JoinInterface& join_module,
                                int first_input_stream_id,
                                int first_input_record_size,
                                int second_input_stream_id,
                                int second_input_record_size,
                                int output_stream_id, int output_record_size) {

  join_module.Reset();
  join_module.DefineOutputStream(
      StreamParameterCalculator::CalculateChunksPerRecord(output_record_size),
      first_input_stream_id, second_input_stream_id, output_stream_id);
  join_module.SetFirstInputStreamChunkCount(
      StreamParameterCalculator::CalculateChunksPerRecord(
          first_input_record_size));
  join_module.SetSecondInputStreamChunkCount(
      StreamParameterCalculator::CalculateChunksPerRecord(
          second_input_record_size));

  SetupTimeMultiplexer(join_module, first_input_record_size,
                       second_input_record_size, output_record_size);

  join_module.StartPrefetchingData();
}

void JoinSetup::SetupTimeMultiplexer(JoinInterface& join_module,
                                     int first_stream_size,
                                     int second_stream_size,
                                     int output_stream_size) {
  int output_chunk_id = 0;
  int data_position = 15;
  for (int first_stream_element_count = 0;
       first_stream_element_count < first_stream_size;
       first_stream_element_count++) {
    join_module.SelectOutputDataElement(
        output_chunk_id, first_stream_element_count / 16, data_position, false);
    data_position--;
    if (data_position == -1) {
      data_position = 15;
      output_chunk_id++;
    }
  }
  // This shift is assumed to be present in the second stream
  int shift_size =
      second_stream_size - (output_stream_size - first_stream_size);

  for (int second_stream_element_count = 0;
       second_stream_element_count < second_stream_size;
       second_stream_element_count++) {
    if (second_stream_element_count >= shift_size) {
      join_module.SelectOutputDataElement(output_chunk_id,
                                          second_stream_element_count / 16,
                                          data_position, true);
      data_position--;
      if (data_position == -1) {
        data_position = 15;
        output_chunk_id++;
      }
    }
  }
}