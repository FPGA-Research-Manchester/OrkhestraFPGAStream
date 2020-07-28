#pragma once
#include <cstdint>

#include "AccelerationModule.hpp"
#include "DMAInterface.hpp"
class DMA : public AccelerationModule, public DMAInterface {
 public:
  ~DMA() override;
  explicit DMA(int* volatile ctrl_ax_ibase_address);

  void setInputControllerParams(int stream_id, int dd_rburst_size,
                                int records_per_ddr_burst, int buffer_start,
                                int buffer_end) override;
  auto getInputControllerParams(int stream_id) -> uint32_t override;
  void setInputControllerStreamAddress(int stream_id,
                                       uintptr_t address) override;
  auto getInputControllerStreamAddress(int stream_id) -> uintptr_t override;
  void setInputControllerStreamSize(int stream_id, int size) override;
  auto getInputControllerStreamSize(int stream_id) -> uint32_t override;
  void startInputController(bool stream_active[16]) override;
  auto isInputControllerFinished() -> bool override;

  void setRecordSize(int stream_id, int record_size) override;
  void setRecordChunkIDs(int stream_id, int interface_cycle,
                         int chunk_id) override;

  void setOutputControllerParams(int stream_id, int dd_rburst_size,
                                 int records_per_ddr_burst, int buffer_start,
                                 int buffer_end) override;
  auto getOutputControllerParams(int stream_id) -> uint32_t override;
  void setOutputControllerStreamAddress(int stream_id,
                                        uintptr_t address) override;
  auto getOutputControllerStreamAddress(int stream_id) -> uintptr_t override;
  void setOutputControllerStreamSize(int stream_id, int size) override;
  auto getOutputControllerStreamSize(int stream_id) -> uint32_t override;
  void startOutputController(bool stream_active[16]) override;
  auto isOutputControllerFinished() -> bool override;

  void setBufferToInterfaceChunk(int stream_id, int clock_cycle, int offset,
                                 int source_chunk4, int source_chunk3,
                                 int source_chunk2, int source_chunk1) override;
  void setBufferToInterfaceSourcePosition(int stream_id, int clock_cycle,
                                          int offset, int source_position4,
                                          int source_position3,
                                          int source_position2,
                                          int source_position1) override;

  void setAXItoBufferChunk(int stream_id, int clock_cycle, int offset,
                           int target_chunk4, int target_chunk3,
                           int target_chunk2, int target_chunk1) override;
  void setAXItoBufferSourcePosition(int stream_id, int clock_cycle, int offset,
                                    int source_position4, int source_position3,
                                    int source_position2,
                                    int source_position1) override;

  void setInterfaceToBufferChunk(int stream_id, int clock_cycle, int offset,
                                 int target_chunk4, int target_chunk3,
                                 int target_chunk2, int target_chunk1) override;
  void setInterfaceToBufferSourcePosition(int stream_id, int clock_cycle,
                                          int offset, int source_position4,
                                          int source_position3,
                                          int source_position2,
                                          int source_position1) override;

  void setBufferToAXIChunk(int stream_id, int clock_cycle, int offset,
                           int source_chunk4, int source_chunk3,
                           int source_chunk2, int source_chunk1) override;
  void setBufferToAXISourcePosition(int stream_id, int clock_cycle, int offset,
                                    int source_position4, int source_position3,
                                    int source_position2,
                                    int source_position1) override;
};
