#include "dma_crossbar_setup.hpp"

#include "dma_crossbar_setup_data.hpp"
#include "query_acceleration_constants.hpp"

void DMACrossbarSetup::CalculateCrossbarSetupData(
    const int& any_chunk, const int& any_position,
    DMASetupData& stream_setup_data, const int& record_size) {
  std::queue<int> source_chunks;
  std::queue<int> target_positions;

  const int last_chunk_leftover_size =
      record_size % query_acceleration_constants::kDatapathWidth;
  const int steps_per_cycle =
      query_acceleration_constants::kDatapathWidth / last_chunk_leftover_size;
  const int cycle_count =
      query_acceleration_constants::kDatapathLength /
      (steps_per_cycle * stream_setup_data.chunks_per_record);

  if (stream_setup_data.is_input_stream) {
    CalculateBufferToInterfaceSetupConfig(
        source_chunks, target_positions, any_chunk, any_position,
        last_chunk_leftover_size, steps_per_cycle, cycle_count,
        stream_setup_data.chunks_per_record);
  } else {
    CalculateInterfaceToBufferSetupConfig(
        source_chunks, target_positions, any_chunk, any_position,
        last_chunk_leftover_size, steps_per_cycle, cycle_count,
        stream_setup_data.chunks_per_record);
  }

  SetCrossbarSetupDataForStream(source_chunks, target_positions,
                                stream_setup_data);
}

void DMACrossbarSetup::CalculateBufferToInterfaceSetupConfig(
    std::queue<int>& source_chunks,
    std::queue<int>& target_positions, const int& any_chunk,
    const int& any_position, const int& last_chunk_leftover_size,
    const int& steps_per_cycle, const int& cycle_count,
    const int& chunks_per_record) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int current_position_shift = cycle_step * last_chunk_leftover_size;
      const int current_offset_point =
          query_acceleration_constants::kDatapathWidth - current_position_shift;
      const int current_cycle_step_initial_chunk =
          cycle_counter * steps_per_cycle + cycle_step + cycle_counter;

      // Initial chunks
      for (int initial_chunk_count = 0;
           initial_chunk_count < chunks_per_record - 1; initial_chunk_count++) {
        const int current_chunk =
            current_cycle_step_initial_chunk + initial_chunk_count;
        // Chunk setting
        // Beginning of the chunk until current_offset_point
        for (int current_position = 0; current_position < current_offset_point;
             current_position++) {
          source_chunks.push(current_chunk);
        }
        // Rest of the chunk
        for (int forward_chunk_position = current_offset_point;
             forward_chunk_position <
             query_acceleration_constants::kDatapathWidth;
             forward_chunk_position++) {
          source_chunks.push(current_chunk + 1);
        }
        // Position setting
        for (int forward_chunk_position = current_offset_point;
             forward_chunk_position <
             query_acceleration_constants::kDatapathWidth;
             forward_chunk_position++) {
          target_positions.push(forward_chunk_position);
        }
        for (int current_chunk_position = 0;
             current_chunk_position < current_offset_point;
             current_chunk_position++) {
          target_positions.push(current_chunk_position);
        }
      }

      // Last chunk chunk setting
      for (int empty_initial_chunk_position = 0;
           empty_initial_chunk_position <
           current_offset_point - last_chunk_leftover_size;
           empty_initial_chunk_position++) {
        source_chunks.push(any_chunk);
      }
      for (int left_over_chunk_position =
               query_acceleration_constants::kDatapathWidth -
               last_chunk_leftover_size;
           left_over_chunk_position <
           query_acceleration_constants::kDatapathWidth;
           left_over_chunk_position++) {
        source_chunks.push(current_cycle_step_initial_chunk +
                           chunks_per_record - 1);
      }
      for (int empty_finishing_chunk_position = current_offset_point;
           empty_finishing_chunk_position <
           query_acceleration_constants::kDatapathWidth;
           empty_finishing_chunk_position++) {
        source_chunks.push(any_chunk);
      }
      // Last chunk position setting
      for (int empty_initial_chunk_position = 0;
           empty_initial_chunk_position <
           query_acceleration_constants::kDatapathWidth -
               last_chunk_leftover_size;
           empty_initial_chunk_position++) {
        target_positions.push(any_position);
      }
      for (int left_over_chunk_position = 0;
           left_over_chunk_position < last_chunk_leftover_size;
           left_over_chunk_position++) {
        target_positions.push(current_offset_point - last_chunk_leftover_size +
                              left_over_chunk_position);
      }
    }
  }
}

void DMACrossbarSetup::CalculateInterfaceToBufferSetupConfig(
    std::queue<int>& source_chunks,
    std::queue<int>& target_positions, const int& any_chunk,
    const int& any_position, const int& last_chunk_leftover_size,
    const int& steps_per_cycle, const int& cycle_count,
    const int& chunks_per_record) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int current_position_shift = cycle_step * last_chunk_leftover_size;
      const int current_offset_point =
          query_acceleration_constants::kDatapathWidth - current_position_shift;
      const int current_cycle_step_initial_chunk =
          cycle_counter * steps_per_cycle + cycle_step + cycle_counter;
      // Initial chunks
      for (int initial_chunk_count = 0;
           initial_chunk_count < chunks_per_record - 1; initial_chunk_count++) {
        const int current_chunk =
            current_cycle_step_initial_chunk + initial_chunk_count;

        // Chunk setting
        // Beginning of the chunk until current_offset_point
        for (int current_position = 0; current_position < current_offset_point;
             current_position++) {
          source_chunks.push(current_chunk);
        }
        // Rest of the chunk (current_position_shift + current_offset_point =
        // datapath_width)
        for (int forward_chunk_position = current_offset_point;
             forward_chunk_position <
             query_acceleration_constants::kDatapathWidth;
             forward_chunk_position++) {
          source_chunks.push(current_chunk + 1);
        }
        // Position setting
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

      // Last chunk
      for (int empty_initial_chunk_position = 0;
           empty_initial_chunk_position <
           current_offset_point - last_chunk_leftover_size;
           empty_initial_chunk_position++) {
        source_chunks.push(any_chunk);
        target_positions.push(any_position);
      }
      for (int left_over_chunk_position =
               query_acceleration_constants::kDatapathWidth -
               last_chunk_leftover_size;
           left_over_chunk_position <
           query_acceleration_constants::kDatapathWidth;
           left_over_chunk_position++) {
        source_chunks.push(current_cycle_step_initial_chunk +
                           chunks_per_record - 1);
        target_positions.push(left_over_chunk_position);
      }
      for (int empty_finishing_chunk_position = current_offset_point;
           empty_finishing_chunk_position <
           query_acceleration_constants::kDatapathWidth;
           empty_finishing_chunk_position++) {
        source_chunks.push(any_chunk);
        target_positions.push(any_position);
      }
    }
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
    stream_setup_data.crossbar_setup_data[current_buffer_chunk] =
        current_chunk_data;
  }
}