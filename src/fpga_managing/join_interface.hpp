#pragma once
class JoinInterface{
 public:
  virtual ~JoinInterface() = default;

  virtual DefineOutputStream(int output_stream_chunk_count, int first_input_stream_id,
                     int second_input_stream_id, int output_stream_id) = 0;
  virtual SetFirstInputStreamChunkCount(int chunk_count) = 0;
  virtual SetSecondInputStreamChunkCount(int chunk_count) = 0;
  virtual SelectNextOutputDataElement(int output_chunk_id, int input_chunk_id,
                                      int data_position,
                                      bool is_element_from_first_stream) = 0;
};