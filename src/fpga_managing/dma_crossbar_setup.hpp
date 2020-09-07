#pragma once
#include <queue>

#include "dma_setup_data.hpp"
class DMACrossbarSetup {
 public:
  static void CalculateCrossbarSetupData(const int& any_chunk,
                                         const int& any_position,
                                         DMASetupData& stream_setup_data,
                                         const int& record_size);

 private:
  static void CalculateInterfaceToBufferSetupConfig(
      std::queue<int>& source_chunks, std::queue<int>& target_positions,
      const int& any_chunk, const int& any_position,
      const int& last_chunk_leftover_size, const int& steps_per_cycle,
      const int& cycle_count, const int& chunks_per_record);
  static void CalculateBufferToInterfaceSetupConfig(
      std::queue<int>& source_chunks, std::queue<int>& target_positions,
      const int& any_chunk, const int& any_position,
      const int& last_chunk_leftover_size, const int& steps_per_cycle,
      const int& cycle_count, const int& chunks_per_record);

  static void InitialDataSetupFromInterfaceToBuffer(
      const int& chunks_per_record, const int& current_cycle_step_initial_chunk,
      const int& current_offset_point, std::queue<int>& source_chunks,
      const int& current_position_shift, std::queue<int>& target_positions);
  static void FinalDataSetupFromInterfaceToBuffer(
      const int& current_offset_point, const int& last_chunk_leftover_size,
      std::queue<int>& source_chunks, const int& any_chunk,
      std::queue<int>& target_positions, const int& any_position,
      const int& current_cycle_step_initial_chunk,
      const int& chunks_per_record);
  
  static void InitialDataSetupFromBufferToInterface(
      const int& chunks_per_record, const int& current_cycle_step_initial_chunk,
      const int& current_offset_point, std::queue<int>& source_chunks,
      std::queue<int>& target_positions);
  static void FinalDataSetupFromBufferToInterface(
      const int& current_offset_point, const int& last_chunk_leftover_size,
      std::queue<int>& source_chunks, const int& any_chunk,
      const int& current_cycle_step_initial_chunk, const int& chunks_per_record,
      std::queue<int>& target_positions, const int& any_position);

  static void InitialChunkSetupFromBufferToInterface(
      const int& current_offset_point, std::queue<int>& source_chunks,
      const int& current_chunk);
  static void InitialPositionSetupFromBufferToInterface(
      const int& current_offset_point, std::queue<int>& target_positions);
  static void FinalPositionSetupFromBufferToInterface(
      const int& last_chunk_leftover_size, std::queue<int>& target_positions,
      const int& any_position, const int& current_offset_point);
  static void FinalChunkSetupFromBufferToInterface(
      const int& current_offset_point, const int& last_chunk_leftover_size,
      std::queue<int>& source_chunks, const int& any_chunk,
      const int& current_cycle_step_initial_chunk,
      const int& chunks_per_record);

  static void InitialChunkSetupFromInterfaceToBuffer(
      const int& current_offset_point, std::queue<int>& source_chunks,
      const int& current_chunk);
  static void InitialPositionSetupFromInterfaceToBuffer(
      const int& current_position_shift, std::queue<int>& target_positions);
  
  static void SetCrossbarSetupDataForStream(std::queue<int>& source_chunks,
                                            std::queue<int>& target_positions,
                                            DMASetupData& stream_setup_data);
};
