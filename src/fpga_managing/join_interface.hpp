#pragma once
/**
 * Interface class to be implemented by #Join
 */
class JoinInterface{
 public:
  virtual ~JoinInterface() = default;

  virtual void DefineOutputStream(int output_stream_chunk_count, int first_input_stream_id,
                     int second_input_stream_id, int output_stream_id) = 0;
  virtual void SetFirstInputStreamChunkCount(int chunk_count) = 0;
  virtual void SetSecondInputStreamChunkCount(int chunk_count) = 0;
  virtual void SelectOutputDataElement(int output_chunk_id, int input_chunk_id,
                                      int data_position,
                                      bool is_element_from_second_stream) = 0;
  virtual void StartPrefetchingData() = 0;
  virtual void Reset() = 0;
};