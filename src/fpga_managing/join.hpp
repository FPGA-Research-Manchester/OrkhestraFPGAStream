#pragma once
#include "acceleration_module.hpp"
#include "join_interface.hpp"

class Join : public AccelerationModule, public JoinInterface {
 private:
 public:
  ~Join() override;
  explicit Join(MemoryManager* memory_manager, int module_position)
      : AccelerationModule(memory_manager, module_position){};

  void DefineOutputStream(int output_stream_chunk_count, int first_input_stream_id, int second_input_stream_id,
                          int output_stream_id) override;
  void SetFirstInputStreamChunkCount(int chunk_count) override;
  void SetSecondInputStreamChunkCount(int chunk_count) override;

  void SelectOutputDataElement(int output_chunk_id, int input_chunk_id,
                                   int data_position,
                                   bool is_element_from_first_stream) override;

  void StartPrefetchingData() override;
};