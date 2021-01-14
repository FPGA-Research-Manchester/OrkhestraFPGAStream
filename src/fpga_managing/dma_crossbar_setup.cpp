#include "dma_crossbar_setup.hpp"

#include <cmath>
#include <iostream>
#include <numeric>

#include "dma_crossbar_setup_data.hpp"
#include "query_acceleration_constants.hpp"

void DMACrossbarSetup::CalculateCrossbarSetupData(
    const int& any_chunk, const int& any_position,
    DMASetupData& stream_setup_data, const int& record_size) {
  std::queue<int> source_chunks;
  std::queue<int> target_positions;

  const int last_chunk_leftover_size =
      record_size % query_acceleration_constants::kDatapathWidth;

  if (last_chunk_leftover_size == 0) {
    for (int chunk_id = 0;
         chunk_id < query_acceleration_constants::kDatapathLength; chunk_id++) {
      for (int position_id = 0;
           position_id < query_acceleration_constants::kDatapathWidth;
           position_id++) {
        source_chunks.push(chunk_id);
        target_positions.push(position_id);
      }
    }
  } else {
    const int steps_per_cycle =
        query_acceleration_constants::kDatapathWidth /
        std::gcd(record_size, query_acceleration_constants::kDatapathWidth);
    const int cycle_count = std::ceil(
        static_cast<float>(query_acceleration_constants::kDatapathLength) /
        static_cast<float>(steps_per_cycle *
                           stream_setup_data.chunks_per_record));

    const float cycle_step_chunk_increase =
        static_cast<float>(record_size) /
        static_cast<float>(query_acceleration_constants::kDatapathWidth);

    if (stream_setup_data.is_input_stream) {
      CalculateBufferToInterfaceSetupConfig(
          source_chunks, target_positions, any_chunk, any_position,
          last_chunk_leftover_size, steps_per_cycle, cycle_count,
          stream_setup_data.chunks_per_record, cycle_step_chunk_increase);
    } else {
      CalculateInterfaceToBufferSetupConfig(
          source_chunks, target_positions, any_chunk, any_position,
          last_chunk_leftover_size, steps_per_cycle, cycle_count,
          stream_setup_data.chunks_per_record, cycle_step_chunk_increase);
    }
  }

  SetCrossbarSetupDataForStream(source_chunks, target_positions,
                                stream_setup_data);
}

void DMACrossbarSetup::CalculateBufferToInterfaceSetupConfig(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& any_chunk, const int& any_position,
    const int& last_chunk_leftover_size, const int& steps_per_cycle,
    const int& cycle_count, const int& chunks_per_record,
    const float& cycle_step_chunk_increase) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int used_leftover_chunks_count =
          cycle_step * last_chunk_leftover_size;
      const int current_offset_point =
          query_acceleration_constants::kDatapathWidth -
          (used_leftover_chunks_count %
              query_acceleration_constants::kDatapathWidth);

      const int current_cycle_step_initial_chunk =
          cycle_step_chunk_increase *
          (cycle_counter * steps_per_cycle + cycle_step);

      InitialDataSetupFromBufferToInterface(
          chunks_per_record, current_cycle_step_initial_chunk,
          current_offset_point, source_chunks, target_positions);

      FinalDataSetupFromBufferToInterface(
          used_leftover_chunks_count,
          current_cycle_step_initial_chunk, 
          current_offset_point, 
          last_chunk_leftover_size,
          chunks_per_record, 
          source_chunks, any_chunk,
          target_positions, any_position);
    }
  }
}

void DMACrossbarSetup::CalculateInterfaceToBufferSetupConfig(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& any_chunk, const int& any_position,
    const int& last_chunk_leftover_size, const int& steps_per_cycle,
    const int& cycle_count, const int& chunks_per_record,
    const float& cycle_step_chunk_increase) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int used_leftover_chunks_count =
          cycle_step * last_chunk_leftover_size;
      const int current_offset_point =
          query_acceleration_constants::kDatapathWidth -
          (used_leftover_chunks_count %
           query_acceleration_constants::kDatapathWidth);

      const int current_cycle_step_initial_chunk =
          cycle_step_chunk_increase *
          (cycle_counter * steps_per_cycle + cycle_step);

      InitialDataSetupFromInterfaceToBuffer(
          chunks_per_record, current_cycle_step_initial_chunk,
          current_offset_point,
          used_leftover_chunks_count %
              query_acceleration_constants::kDatapathWidth,
          source_chunks,
          target_positions);

      FinalDataSetupFromInterfaceToBuffer(
          used_leftover_chunks_count,
          current_cycle_step_initial_chunk,  
          last_chunk_leftover_size,
          chunks_per_record, 
          source_chunks, any_chunk,
          target_positions, any_position);
    }
  }
}

void DMACrossbarSetup::InitialDataSetupFromBufferToInterface(
    const int& chunks_per_record, const int& current_cycle_step_initial_chunk,
    const int& current_offset_point, std::queue<int>& source_chunks,
    std::queue<int>& target_positions) {
  for (int initial_chunk_count = 0; initial_chunk_count < chunks_per_record - 1;
       initial_chunk_count++) {
    const int current_chunk =
        current_cycle_step_initial_chunk + initial_chunk_count;
    InitialChunkSetupFromBufferToInterface(current_offset_point, source_chunks,
                                           current_chunk);
    InitialPositionSetupFromBufferToInterface(current_offset_point,
                                              target_positions);
  }
}

void DMACrossbarSetup::FinalDataSetupFromBufferToInterface(
    const int& used_leftover_chunks_count,
    const int& current_cycle_step_initial_chunk,
    const int& current_offset_point, const int& last_chunk_leftover_size,
    const int& chunks_per_record,
    std::queue<int>& source_chunks, const int& any_chunk,
    std::queue<int>& target_positions, const int& any_position) {
  FinalChunkSetupFromBufferToInterface(
      used_leftover_chunks_count, last_chunk_leftover_size,
      current_cycle_step_initial_chunk, chunks_per_record, source_chunks,
      any_chunk);
  FinalPositionSetupFromBufferToInterface(last_chunk_leftover_size,
                                          current_offset_point,
                                          target_positions, any_position);
}

void DMACrossbarSetup::InitialDataSetupFromInterfaceToBuffer(
    const int& chunks_per_record, const int& current_cycle_step_initial_chunk,
    const int& current_offset_point, const int& current_position_shift, 
    std::queue<int>& source_chunks,
    std::queue<int>& target_positions) {
  for (int initial_chunk_count = 0; initial_chunk_count < chunks_per_record - 1;
       initial_chunk_count++) {
    const int current_chunk =
        current_cycle_step_initial_chunk + initial_chunk_count;

    InitialChunkSetupFromInterfaceToBuffer(current_offset_point, source_chunks,
                                           current_chunk);
    InitialPositionSetupFromInterfaceToBuffer(current_position_shift,
                                              target_positions);
  }
}

void DMACrossbarSetup::FinalDataSetupFromInterfaceToBuffer(
    const int& used_leftover_chunks_count,
    const int& current_cycle_step_initial_chunk,
    const int& last_chunk_leftover_size,
    const int& chunks_per_record, std::queue<int>& source_chunks,
    const int& any_chunk, std::queue<int>& target_positions,
    const int& any_position) {
  // Chunks and positions are set in the same fashion from interface to buffer
  const int initial_any_chunk_sequence_size =
      query_acceleration_constants::kDatapathWidth -
      (used_leftover_chunks_count %
       query_acceleration_constants::kDatapathWidth) -
      last_chunk_leftover_size;

  int initial_valid_chunk_sequence_size = 0;
  int last_any_chunk_sequence_size = 0;
  int last_valid_chunk_sequence_size = 0;

  if (initial_any_chunk_sequence_size < 0) {
    initial_valid_chunk_sequence_size =
        last_chunk_leftover_size + initial_any_chunk_sequence_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth - last_chunk_leftover_size;
    last_valid_chunk_sequence_size = std::abs(initial_any_chunk_sequence_size);
  } else {
    initial_valid_chunk_sequence_size = last_chunk_leftover_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size - initial_any_chunk_sequence_size;
    last_valid_chunk_sequence_size = 0;
  }

  for (int initial_any_chunk_index = 0;
       initial_any_chunk_index < initial_any_chunk_sequence_size;
       initial_any_chunk_index++) {
    source_chunks.push(any_chunk);
    target_positions.push(any_position);
  }
  for (int initial_valid_chunk_index = 0;
       initial_valid_chunk_index < initial_valid_chunk_sequence_size;
       initial_valid_chunk_index++) {
    source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record -
                       1);
    const int current_target_position =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size +
        last_valid_chunk_sequence_size + initial_valid_chunk_index;
    target_positions.push(current_target_position);
  }
  for (int last_any_chunk_index = 0;
       last_any_chunk_index < last_any_chunk_sequence_size;
       last_any_chunk_index++) {
    source_chunks.push(any_chunk);
    target_positions.push(any_position);
  }
  for (int last_valid_chunk_index = 0;
       last_valid_chunk_index < last_valid_chunk_sequence_size;
       last_valid_chunk_index++) {
    source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record);
    const int current_target_position =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size +
        +last_valid_chunk_index;
    target_positions.push(current_target_position);
  }
}

void DMACrossbarSetup::FinalPositionSetupFromBufferToInterface(
    const int& last_chunk_leftover_size, const int& current_offset_point,
    std::queue<int>& target_positions, const int& any_position) {
  for (int empty_initial_chunk_position = 0;
       empty_initial_chunk_position <
       query_acceleration_constants::kDatapathWidth - last_chunk_leftover_size;
       empty_initial_chunk_position++) {
    target_positions.push(any_position);
  }
  for (int left_over_chunk_position = 0;
       left_over_chunk_position < last_chunk_leftover_size;
       left_over_chunk_position++) {
    const int current_position = current_offset_point -
                                 last_chunk_leftover_size +
                                 left_over_chunk_position;
    if (current_position < 0) {
      target_positions.push(query_acceleration_constants::kDatapathWidth +
                            current_position);
    } else {
      target_positions.push(current_position);
    }
  }
}

void DMACrossbarSetup::FinalChunkSetupFromBufferToInterface(
    const int& used_leftover_chunks_count, const int& last_chunk_leftover_size,
    const int& current_cycle_step_initial_chunk, const int& chunks_per_record,
    std::queue<int>& source_chunks, const int& any_chunk) {

  const int initial_any_chunk_sequence_size =
      query_acceleration_constants::kDatapathWidth -
      (used_leftover_chunks_count %
          query_acceleration_constants::kDatapathWidth) -
      last_chunk_leftover_size;

  int initial_valid_chunk_sequence_size = 0;
  int last_any_chunk_sequence_size = 0;
  int last_valid_chunk_sequence_size = 0;

  if (initial_any_chunk_sequence_size < 0) {
    initial_valid_chunk_sequence_size =
        last_chunk_leftover_size + initial_any_chunk_sequence_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth - last_chunk_leftover_size;
    last_valid_chunk_sequence_size =
        std::abs(initial_any_chunk_sequence_size);
  } else {
    initial_valid_chunk_sequence_size = last_chunk_leftover_size;
    last_any_chunk_sequence_size =
        query_acceleration_constants::kDatapathWidth -
        last_chunk_leftover_size - initial_any_chunk_sequence_size;
    last_valid_chunk_sequence_size = 0;
  }
  
  for (int initial_any_chunk_index = 0;
       initial_any_chunk_index < initial_any_chunk_sequence_size;
       initial_any_chunk_index++) {
    source_chunks.push(any_chunk);
  }
  for (int initial_valid_chunk_index = 0;
       initial_valid_chunk_index < initial_valid_chunk_sequence_size;
       initial_valid_chunk_index++) {
    source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record -
                       1);
  }
  for (int last_any_chunk_index = 0;
       last_any_chunk_index < last_any_chunk_sequence_size;
       last_any_chunk_index++) {
    source_chunks.push(any_chunk);
  }
  for (int last_valid_chunk_index = 0;
       last_valid_chunk_index < last_valid_chunk_sequence_size;
       last_valid_chunk_index++) {
    source_chunks.push(current_cycle_step_initial_chunk + chunks_per_record);
  }
}

void DMACrossbarSetup::InitialPositionSetupFromBufferToInterface(
    const int& current_offset_point, std::queue<int>& target_positions) {
  for (int forward_chunk_position = current_offset_point;
       forward_chunk_position < query_acceleration_constants::kDatapathWidth;
       forward_chunk_position++) {
    target_positions.push(forward_chunk_position);
  }
  for (int current_chunk_position = 0;
       current_chunk_position < current_offset_point;
       current_chunk_position++) {
    target_positions.push(current_chunk_position);
  }
}

void DMACrossbarSetup::InitialChunkSetupFromBufferToInterface(
    const int& current_offset_point, std::queue<int>& source_chunks,
    const int& current_chunk) {
  // Beginning of the chunk until current_offset_point
  for (int current_position = 0; current_position < current_offset_point;
       current_position++) {
    source_chunks.push(current_chunk);
  }
  // Rest of the chunk
  for (int forward_chunk_position = current_offset_point;
       forward_chunk_position < query_acceleration_constants::kDatapathWidth;
       forward_chunk_position++) {
    source_chunks.push(current_chunk + 1);
  }
}

void DMACrossbarSetup::InitialPositionSetupFromInterfaceToBuffer(
    const int& current_position_shift, std::queue<int>& target_positions) {
  for (int current_position = current_position_shift;
       current_position < query_acceleration_constants::kDatapathWidth;
       current_position++) {
    target_positions.push(current_position);
  }
  for (int forward_chunk_position = 0;
       forward_chunk_position < current_position_shift;
       forward_chunk_position++) {
    target_positions.push(forward_chunk_position);
  }
}

void DMACrossbarSetup::InitialChunkSetupFromInterfaceToBuffer(
    const int& current_offset_point, std::queue<int>& source_chunks,
    const int& current_chunk) {
  // Beginning of the chunk until current_offset_point
  for (int current_position = 0; current_position < current_offset_point;
       current_position++) {
    source_chunks.push(current_chunk);
  }
  // Rest of the chunk (current_position_shift + current_offset_point =
  // datapath_width)
  for (int forward_chunk_position = current_offset_point;
       forward_chunk_position < query_acceleration_constants::kDatapathWidth;
       forward_chunk_position++) {
    source_chunks.push(current_chunk + 1);
  }
}

void DMACrossbarSetup::SetCrossbarSetupDataForStream(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    DMASetupData& stream_setup_data) {
  for (int current_buffer_chunk = 0;
       current_buffer_chunk < query_acceleration_constants::kDatapathLength;
       current_buffer_chunk++) {
    DMACrossbarSetupData current_chunk_data;
    for (int current_data_input = 0;
         current_data_input < query_acceleration_constants::kDatapathWidth;
         current_data_input++) {
      current_chunk_data.chunk_data[current_data_input] = source_chunks.front();
      source_chunks.pop();
      current_chunk_data.position_data[current_data_input] =
          target_positions.front();
      target_positions.pop();
    }
    stream_setup_data.crossbar_setup_data.push_back(
        current_chunk_data);
  }
}