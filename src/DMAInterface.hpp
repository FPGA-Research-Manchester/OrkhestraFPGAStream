#pragma once
#include <cstdint>
class DMAInterface {
 public:
  virtual ~DMAInterface() = default;

  virtual void SetInputControllerParams(int stream_id, int dd_rburst_size,
                                        int records_per_ddr_burst,
                                        int buffer_start, int buffer_end) = 0;
  virtual auto GetInputControllerParams(int stream_id) -> uint32_t = 0;
  virtual void SetInputControllerStreamAddress(int stream_id,
                                               uintptr_t address) = 0;
  virtual auto GetInputControllerStreamAddress(int stream_id) -> uintptr_t = 0;
  virtual void SetInputControllerStreamSize(int stream_id, int size) = 0;
  virtual auto GetInputControllerStreamSize(int stream_id) -> uint32_t = 0;
  virtual void StartInputController(bool stream_active[16]) = 0;
  virtual auto IsInputControllerFinished() -> bool = 0;

  virtual void SetRecordSize(int stream_id, int record_size) = 0;
  virtual void SetRecordChunkIDs(int stream_id, int interface_cycle,
                                 int chunk_id) = 0;

  virtual void SetOutputControllerParams(int stream_id, int dd_rburst_size,
                                         int records_per_ddr_burst,
                                         int buffer_start, int buffer_end) = 0;
  virtual auto GetOutputControllerParams(int stream_id) -> uint32_t = 0;
  virtual void SetOutputControllerStreamAddress(int stream_id,
                                                uintptr_t address) = 0;
  virtual auto GetOutputControllerStreamAddress(int stream_id) -> uintptr_t = 0;
  virtual void SetOutputControllerStreamSize(int stream_id, int size) = 0;
  virtual auto GetOutputControllerStreamSize(int stream_id) -> uint32_t = 0;
  virtual void StartOutputController(bool stream_active[16]) = 0;
  virtual auto IsOutputControllerFinished() -> bool = 0;

  virtual void SetBufferToInterfaceChunk(int stream_id, int clock_cycle,
                                         int offset, int source_chunk4,
                                         int source_chunk3, int source_chunk2,
                                         int source_chunk1) = 0;
  virtual void SetBufferToInterfaceSourcePosition(
      int stream_id, int clock_cycle, int offset, int source_position4,
      int source_position3, int source_position2, int source_position1) = 0;

  virtual void SetAXItoBufferChunk(int stream_id, int clock_cycle, int offset,
                                   int target_chunk4, int target_chunk3,
                                   int target_chunk2, int target_chunk1) = 0;
  virtual void SetAXItoBufferSourcePosition(int stream_id, int clock_cycle,
                                            int offset, int source_position4,
                                            int source_position3,
                                            int source_position2,
                                            int source_position1) = 0;

  virtual void SetInterfaceToBufferChunk(int stream_id, int clock_cycle,
                                         int offset, int target_chunk4,
                                         int target_chunk3, int target_chunk2,
                                         int target_chunk1) = 0;
  virtual void SetInterfaceToBufferSourcePosition(
      int stream_id, int clock_cycle, int offset, int source_position4,
      int source_position3, int source_position2, int source_position1) = 0;

  virtual void SetBufferToAXIChunk(int stream_id, int clock_cycle, int offset,
                                   int source_chunk4, int source_chunk3,
                                   int source_chunk2, int source_chunk1) = 0;
  virtual void SetBufferToAXISourcePosition(int stream_id, int clock_cycle,
                                            int offset, int source_position4,
                                            int source_position3,
                                            int source_position2,
                                            int source_position1) = 0;
};