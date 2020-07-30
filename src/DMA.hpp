#pragma once
#include <cstdint>

#include "AccelerationModule.hpp"
#include "DMAInterface.hpp"
class DMA : public AccelerationModule, public DMAInterface {
 public:
  ~DMA() override;
  explicit DMA(int* volatile ctrl_axi_base_address);

  void SetInputControllerParams(int stream_id, int dd_rburst_size,
                                int records_per_ddr_burst, int buffer_start,
                                int buffer_end) override;
  auto GetInputControllerParams(int stream_id) -> uint32_t override;
  void SetInputControllerStreamAddress(int stream_id,
                                       uintptr_t address) override;
  auto GetInputControllerStreamAddress(int stream_id) -> uintptr_t override;
  void SetInputControllerStreamSize(int stream_id, int size) override;
  auto GetInputControllerStreamSize(int stream_id) -> uint32_t override;
  void StartInputController(bool stream_active[16]) override;
  auto IsInputControllerFinished() -> bool override;

  void SetRecordSize(int stream_id, int record_size) override;
  void SetRecordChunkIDs(int stream_id, int interface_cycle,
                         int chunk_id) override;

  void SetOutputControllerParams(int stream_id, int dd_rburst_size,
                                 int records_per_ddr_burst, int buffer_start,
                                 int buffer_end) override;
  auto GetOutputControllerParams(int stream_id) -> uint32_t override;
  void SetOutputControllerStreamAddress(int stream_id,
                                        uintptr_t address) override;
  auto GetOutputControllerStreamAddress(int stream_id) -> uintptr_t override;
  void SetOutputControllerStreamSize(int stream_id, int size) override;
  auto GetOutputControllerStreamSize(int stream_id) -> uint32_t override;
  void StartOutputController(bool stream_active[16]) override;
  auto IsOutputControllerFinished() -> bool override;

  void SetBufferToInterfaceChunk(int stream_id, int clock_cycle, int offset,
                                 int source_chunk4, int source_chunk3,
                                 int source_chunk2, int source_chunk1) override;
  void SetBufferToInterfaceSourcePosition(int stream_id, int clock_cycle,
                                          int offset, int source_position4,
                                          int source_position3,
                                          int source_position2,
                                          int source_position1) override;

  void SetAXItoBufferChunk(int stream_id, int clock_cycle, int offset,
                           int target_chunk4, int target_chunk3,
                           int target_chunk2, int target_chunk1) override;
  void SetAXItoBufferSourcePosition(int stream_id, int clock_cycle, int offset,
                                    int source_position4, int source_position3,
                                    int source_position2,
                                    int source_position1) override;

  void SetInterfaceToBufferChunk(int stream_id, int clock_cycle, int offset,
                                 int target_chunk4, int target_chunk3,
                                 int target_chunk2, int target_chunk1) override;
  void SetInterfaceToBufferSourcePosition(int stream_id, int clock_cycle,
                                          int offset, int source_position4,
                                          int source_position3,
                                          int source_position2,
                                          int source_position1) override;

  void SetBufferToAXIChunk(int stream_id, int clock_cycle, int offset,
                           int source_chunk4, int source_chunk3,
                           int source_chunk2, int source_chunk1) override;
  void SetBufferToAXISourcePosition(int stream_id, int clock_cycle, int offset,
                                    int source_position4, int source_position3,
                                    int source_position2,
                                    int source_position1) override;
};
