#include "DMACrossbarSetup.hpp"

#include "DMACrossbarSetupData.hpp"

void DMACrossbarSetup::FindOutputCrossbarSetupData(
    const int& ANY_CHUNK, const int& ANY_POSITION,
    DMASetupData& outputStreamSetupData) {
  std::queue<int> source_chunks;
  std::queue<int> target_positions;
  CalculateInterfaceToBufferSetupConfig(source_chunks, target_positions,
                                        ANY_CHUNK, ANY_POSITION);
  SetCrossbarSetupDataForStream(source_chunks, target_positions,
                                outputStreamSetupData);
}

void DMACrossbarSetup::CalculateInterfaceToBufferSetupConfig(
    std::queue<int>& sourceChunks, std::queue<int>& targetPositions,
    const int& ANY_CHUNK, const int& ANY_POSITION) {
  for (int cycle_counter = 0; cycle_counter < 2; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < 8; cycle_step++) {
      // Initial chunk
      for (int forward_chunk_counter = 16 - cycle_step * 2;
           forward_chunk_counter < 16; forward_chunk_counter++) {
        sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
        targetPositions.push(15 - forward_chunk_counter);
      }
      for (int current_chunk_counter = 0;
           current_chunk_counter < 16 - cycle_step * 2;
           current_chunk_counter++) {
        sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter));
        targetPositions.push(15 - current_chunk_counter);
      }
      // Last chunk
      sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      for (int empty_initial_chunk_counter = 0;
           empty_initial_chunk_counter < cycle_step * 2;
           empty_initial_chunk_counter++) {
        sourceChunks.push(ANY_CHUNK);
        targetPositions.push(ANY_POSITION);
      }
      targetPositions.push(15 - 0);
      targetPositions.push(15 - 1);
      for (int empty_finishing_chunk_counter = cycle_step * 2 + 2;
           empty_finishing_chunk_counter < 16;
           empty_finishing_chunk_counter++) {
        sourceChunks.push(ANY_CHUNK);
        targetPositions.push(ANY_POSITION);
      }
    }
  }
}

void DMACrossbarSetup::FindInputCrossbarSetupData(
    const int& ANY_CHUNK, const int& ANY_POSITION,
    DMASetupData& inputStreamSetupData) {
  std::queue<int> source_chunks;
  std::queue<int> target_positions;
  CalculateBufferToInterfaceSetupConfig(source_chunks, target_positions,
                                        ANY_CHUNK, ANY_POSITION);
  SetCrossbarSetupDataForStream(source_chunks, target_positions,
                                inputStreamSetupData);
}

void DMACrossbarSetup::CalculateBufferToInterfaceSetupConfig(
    std::queue<int>& sourceChunks, std::queue<int>& targetPositions,
    const int& ANY_CHUNK, const int& ANY_POSITION) {
  for (int cycle_counter = 0; cycle_counter < 2; cycle_counter++) {
    for (int cycle_step = 0; cycle_step < 8; cycle_step++) {
      // Initial chunk
      for (int forward_chunk_counter = 0;
           forward_chunk_counter < cycle_step * 2; forward_chunk_counter++) {
        sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      }
      for (int current_chunk_counter = cycle_step * 2;
           current_chunk_counter < 16; current_chunk_counter++) {
        sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter));
        targetPositions.push(15 - current_chunk_counter);
      }
      for (int forward_chunk_counter = 0;
           forward_chunk_counter < cycle_step * 2; forward_chunk_counter++) {
        targetPositions.push(15 - forward_chunk_counter);
      }
      // Last chunk
      targetPositions.push(15 - cycle_step * 2);
      targetPositions.push(15 - (cycle_step * 2 + 1));
      for (int empty_initial_chunk_counter = 0;
           empty_initial_chunk_counter < cycle_step * 2;
           empty_initial_chunk_counter++) {
        sourceChunks.push(ANY_CHUNK);
        targetPositions.push(ANY_POSITION);
      }
      sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      sourceChunks.push((cycle_step + 8 * cycle_counter + cycle_counter) + 1);
      for (int empty_finishing_chunk_counter = cycle_step * 2 + 2;
           empty_finishing_chunk_counter < 16;
           empty_finishing_chunk_counter++) {
        sourceChunks.push(ANY_CHUNK);
        targetPositions.push(ANY_POSITION);
      }
    }
  }
}

void DMACrossbarSetup::SetCrossbarSetupDataForStream(
    std::queue<int>& sourceChunks, std::queue<int>& targetPositions,
    DMASetupData& streamSetupData) {
  for (int current_buffer_chunk = 0; current_buffer_chunk < 32;
       current_buffer_chunk++) {
    DMACrossbarSetupData current_chunk_data;
    for (int current_data_input = 0; current_data_input < 16;
         current_data_input++) {
      current_chunk_data.chunkData[current_data_input] = sourceChunks.front();
      sourceChunks.pop();
      current_chunk_data.positionData[current_data_input] =
          targetPositions.front();
      targetPositions.pop();
    }
    streamSetupData.crossbarSetupData[current_buffer_chunk] =
        current_chunk_data;
  }
}