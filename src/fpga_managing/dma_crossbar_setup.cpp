#include "dma_crossbar_setup.hpp"

#include "dma_crossbar_setup_data.hpp"

const int last_chunk_leftover_size = 2;
const int cycle_count = 2;
const int steps_per_cycle = 8;
const int datapath_width = 16;

void DMACrossbarSetup::FindOutputCrossbarSetupData(
    const int& any_chunk, const int& any_position,
    DMASetupData& output_stream_setup_data) {
  std::queue<int> source_chunks;
  std::queue<int> target_positions;
  CalculateInterfaceToBufferSetupConfig(source_chunks, target_positions,
                                        any_chunk, any_position);
  SetCrossbarSetupDataForStream(source_chunks, target_positions,
                                output_stream_setup_data);
}

// TODO: Add chunk count for loop for both input and output
void DMACrossbarSetup::CalculateInterfaceToBufferSetupConfig(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& any_chunk, const int& any_position) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int current_position_shift = cycle_step * last_chunk_leftover_size;
      const int current_offset_point = datapath_width - current_position_shift;
      const int current_chunk =
          cycle_counter * steps_per_cycle + cycle_step + cycle_counter;

      // Initial chunk chunk setting
      // Beginning of the chunk until current_offset_point
      for (int current_position = 0; current_position < current_offset_point;
           current_position++) {
        source_chunks.push(current_chunk);
      }
      // Rest of the chunk (current_position_shift + current_offset_point =
      // datapath_width)
      for (int forward_chunk_position = current_offset_point;
           forward_chunk_position < datapath_width; forward_chunk_position++) {
        source_chunks.push(current_chunk + 1);
      }
      // Initial chunk position setting
      for (int current_position = current_position_shift;
           current_position < datapath_width; current_position++) {
        target_positions.push(current_position);
      }
      for (int forward_chunk_position = 0;
           forward_chunk_position < current_position_shift;
           forward_chunk_position++) {
        target_positions.push(forward_chunk_position);
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
               datapath_width - last_chunk_leftover_size;
           left_over_chunk_position < datapath_width;
           left_over_chunk_position++) {
        source_chunks.push(current_chunk + 1);
        target_positions.push(left_over_chunk_position);
      }
      for (int empty_finishing_chunk_position = current_offset_point;
           empty_finishing_chunk_position < datapath_width;
           empty_finishing_chunk_position++) {
        source_chunks.push(any_chunk);
        target_positions.push(any_position);
      }
    }
  }
}

void DMACrossbarSetup::FindInputCrossbarSetupData(
    const int& any_chunk, const int& any_position,
    DMASetupData& input_stream_setup_data) {
  std::queue<int> source_chunks;
  std::queue<int> target_positions;
  CalculateBufferToInterfaceSetupConfig(source_chunks, target_positions,
                                        any_chunk, any_position);
  SetCrossbarSetupDataForStream(source_chunks, target_positions,
                                input_stream_setup_data);
}

void DMACrossbarSetup::CalculateBufferToInterfaceSetupConfig(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& any_chunk, const int& any_position) {
  for (int cycle_counter = 0; cycle_counter < cycle_count; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < steps_per_cycle; cycle_step++) {
      const int current_position_shift = cycle_step * last_chunk_leftover_size;
      const int current_offset_point = datapath_width - current_position_shift;
      const int current_chunk =
          cycle_counter * steps_per_cycle + cycle_step + cycle_counter;
      // Initial chunk chunk setting
      // Beginning of the chunk until current_offset_point
      for (int current_position = 0; current_position < current_offset_point;
           current_position++) {
        source_chunks.push(current_chunk);
      }
      // Rest of the chunk
      for (int forward_chunk_position = current_offset_point;
           forward_chunk_position < datapath_width; forward_chunk_position++) {
        source_chunks.push(current_chunk + 1);
      }
      // Initial chunk position setting
      for (int forward_chunk_position = current_offset_point;
           forward_chunk_position < datapath_width; forward_chunk_position++) {
        target_positions.push(forward_chunk_position);
      }
      for (int current_chunk_position = 0;
           current_chunk_position < current_offset_point;
           current_chunk_position++) {
        target_positions.push(current_chunk_position);
      }

      // Last chunk chunk setting
      for (int empty_initial_chunk_position = 0;
           empty_initial_chunk_position <
           current_offset_point - last_chunk_leftover_size;
           empty_initial_chunk_position++) {
        source_chunks.push(any_chunk);
      }
      for (int left_over_chunk_position =
               datapath_width - last_chunk_leftover_size;
           left_over_chunk_position < datapath_width;
           left_over_chunk_position++) {
        source_chunks.push(current_chunk + 1);
      }
      for (int empty_finishing_chunk_position = current_offset_point;
           empty_finishing_chunk_position < datapath_width;
           empty_finishing_chunk_position++) {
        source_chunks.push(any_chunk);
      }
      // Last chunk position setting
      for (int empty_initial_chunk_position = 0;
           empty_initial_chunk_position <
           datapath_width - last_chunk_leftover_size;
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

void DMACrossbarSetup::SetCrossbarSetupDataForStream(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    DMASetupData& stream_setup_data) {
  for (int current_buffer_chunk = 0; current_buffer_chunk < 32;
       current_buffer_chunk++) {
    DMACrossbarSetupData current_chunk_data;
    for (int current_data_input = 0; current_data_input < 16;
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