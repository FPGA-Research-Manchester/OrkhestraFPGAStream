#include "join.hpp"

#include <cmath>

Join::~Join() = default;

void Join::DefineOutputStream(int output_stream_chunk_count,
                              int first_input_stream_id,
                              int second_input_stream_id,
                              int output_stream_id) {
  AccelerationModule::WriteToModule(4, ((output_stream_chunk_count - 1) << 24) +
                                           (second_input_stream_id << 16) +
                                           (first_input_stream_id << 8) +
                                           output_stream_id);
}

void Join::SetFirstInputStreamChunkCount(int chunk_count) {
  AccelerationModule::WriteToModule(8, static_cast<int>(log2(chunk_count)));
}

void Join::SetSecondInputStreamChunkCount(int chunk_count) {
  AccelerationModule::WriteToModule(12, static_cast<int>(log2(chunk_count)));
}

void Join::SelectOutputDataElement(int output_chunk_id, int input_chunk_id,
                                   int data_position,
                                   bool is_element_from_second_stream) {
  AccelerationModule::WriteToModule(
      (1 << 13) + (output_chunk_id << 7) + (data_position << 2),
      (static_cast<int>(is_element_from_second_stream) << 16) + input_chunk_id);
}

void Join::StartPrefetchingData() { AccelerationModule::WriteToModule(0, 1); }
