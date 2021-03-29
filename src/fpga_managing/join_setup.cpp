#include "join_setup.hpp"

#include "query_acceleration_constants.hpp"
#include "stream_parameter_calculator.hpp"

void JoinSetup::SetupJoinModule(JoinInterface& join_module,
                                int first_input_stream_id,
                                int first_input_record_size,
                                int second_input_stream_id,
                                int second_input_record_size,
                                int output_stream_id, int output_chunks_per_record, int shift_size) {

  join_module.Reset();
  join_module.DefineOutputStream(output_chunks_per_record,
      first_input_stream_id, second_input_stream_id, output_stream_id);
  join_module.SetFirstInputStreamChunkCount(
      StreamParameterCalculator::CalculateChunksPerRecord(
          first_input_record_size));
  join_module.SetSecondInputStreamChunkCount(
      StreamParameterCalculator::CalculateChunksPerRecord(
          second_input_record_size));

  SetupTimeMultiplexer(join_module, first_input_record_size,
                       second_input_record_size, shift_size);

  join_module.StartPrefetchingData();
}

void JoinSetup::SetupTimeMultiplexer(JoinInterface& join_module,
                                     int first_stream_size,
                                     int second_stream_size, int shift_size) {
  int output_chunk_id = 0;
  int data_position = query_acceleration_constants::kDatapathWidth - 1;
  for (int first_stream_element_count = 0;
       first_stream_element_count < first_stream_size;
       first_stream_element_count++) {
    join_module.SelectOutputDataElement(
        output_chunk_id,
        first_stream_element_count /
            query_acceleration_constants::kDatapathWidth,
        data_position, false);
    data_position--;
    if (data_position == -1) {
      data_position = query_acceleration_constants::kDatapathWidth - 1;
      output_chunk_id++;
    }
  }

  for (int second_stream_element_count = 0;
       second_stream_element_count < second_stream_size;
       second_stream_element_count++) {
    if (second_stream_element_count >= shift_size) {
      join_module.SelectOutputDataElement(output_chunk_id,
          second_stream_element_count /
              query_acceleration_constants::kDatapathWidth,
                                          data_position, true);
      data_position--;
      if (data_position == -1) {
        data_position = query_acceleration_constants::kDatapathWidth - 1;
        output_chunk_id++;
      }
    }
  }
}