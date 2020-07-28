#include "DMACrossbarSetup.hpp"

#include "DMACrossbarSetupData.hpp"

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

void DMACrossbarSetup::CalculateInterfaceToBufferSetupConfig(
    std::queue<int>& source_chunks, std::queue<int>& target_positions,
    const int& any_chunk, const int& any_position) {
  for (int cycle_counter = 0; cycle_counter < 2; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < 8; cycle_step++) {
      // Initial chunk
      for (int forward_chunk_counter = 16 - cycle_step * 2;
           forward_chunk_counter < 16; forward_chunk_counter++) {
        source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter) +
                           1);
        target_positions.push(15 - forward_chunk_counter);
      }
      for (int current_chunk_counter = 0;
           current_chunk_counter < 16 - cycle_step * 2;
           current_chunk_counter++) {
        source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter));
        target_positions.push(15 - current_chunk_counter);
      }
      // Last chunk
      source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      for (int empty_initial_chunk_counter = 0;
           empty_initial_chunk_counter < cycle_step * 2;
           empty_initial_chunk_counter++) {
        source_chunks.push(any_chunk);
        target_positions.push(any_position);
      }
      target_positions.push(15 - 0);
      target_positions.push(15 - 1);
      for (int empty_finishing_chunk_counter = cycle_step * 2 + 2;
           empty_finishing_chunk_counter < 16;
           empty_finishing_chunk_counter++) {
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
  for (int cycle_counter = 0; cycle_counter < 2; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < 8; cycle_step++) {
      // Initial chunk
      for (int forward_chunk_counter = 0;
           forward_chunk_counter < cycle_step * 2; forward_chunk_counter++) {
        source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter) +
                           1);
      }
      for (int current_chunk_counter = cycle_step * 2;
           current_chunk_counter < 16; current_chunk_counter++) {
        source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter));
        target_positions.push(15 - current_chunk_counter);
      }
      for (int forward_chunk_counter = 0;
           forward_chunk_counter < cycle_step * 2; forward_chunk_counter++) {
        target_positions.push(15 - forward_chunk_counter);
      }
      // Last chunk
      target_positions.push(15 - cycle_step * 2);
      target_positions.push(15 - (cycle_step * 2 + 1));
      for (int empty_initial_chunk_counter = 0;
           empty_initial_chunk_counter < cycle_step * 2;
           empty_initial_chunk_counter++) {
        source_chunks.push(any_chunk);
        target_positions.push(any_position);
      }
      source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      source_chunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      for (int empty_finishing_chunk_counter = cycle_step * 2 + 2;
           empty_finishing_chunk_counter < 16;
           empty_finishing_chunk_counter++) {
        source_chunks.push(any_chunk);
        target_positions.push(any_position);
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
      current_chunk_data.chunkData[current_data_input] = source_chunks.front();
      source_chunks.pop();
      current_chunk_data.positionData[current_data_input] =
          target_positions.front();
      target_positions.pop();
    }
    stream_setup_data.crossbarSetupData[current_buffer_chunk] =
        current_chunk_data;
  }
}