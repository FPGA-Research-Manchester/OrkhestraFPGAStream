#pragma once
#include <cstdint>
class DMAInterface {
 public:
  virtual ~DMAInterface() = default;

  virtual void setInputControllerParams(int stream_id, int dd_rburst_size,
                                        int records_per_ddr_burst,
                                        int buffer_start, int buffer_end) = 0;
  virtual auto getInputControllerParams(int stream_id) -> uint32_t = 0;
  virtual void setInputControllerStreamAddress(int stream_id,
                                               uintptr_t address) = 0;
  virtual auto getInputControllerStreamAddress(int stream_id) -> uintptr_t = 0;
  virtual void setInputControllerStreamSize(int stream_id, int size) = 0;
  virtual auto getInputControllerStreamSize(int stream_id) -> uint32_t = 0;
  virtual void startInputController(bool stream_active[16]) = 0;
  virtual auto isInputControllerFinished() -> bool = 0;

  virtual void setRecordSize(int stream_id, int record_size) = 0;
  virtual void setRecordChunkIDs(int stream_id, int interface_cycle,
                                 int chunk_id) = 0;

  virtual void setOutputControllerParams(int stream_id, int dd_rburst_size,
                                         int records_per_ddr_burst,
                                         int buffer_start, int buffer_end) = 0;
  virtual auto getOutputControllerParams(int stream_id) -> uint32_t = 0;
  virtual void setOutputControllerStreamAddress(int stream_id,
                                                uintptr_t address) = 0;
  virtual auto getOutputControllerStreamAddress(int stream_id) -> uintptr_t = 0;
  virtual void setOutputControllerStreamSize(int stream_id, int size) = 0;
  virtual auto getOutputControllerStreamSize(int stream_id) -> uint32_t = 0;
  virtual void startOutputController(bool stream_active[16]) = 0;
  virtual auto isOutputControllerFinished() -> bool = 0;

  virtual void setBufferToInterfaceChunk(int stream_id, int clock_cycle,
                                         int offset, int source_chunk4,
                                         int source_chunk3, int source_chunk2,
                                         int source_chunk1) = 0;
  virtual void setBufferToInterfaceSourcePosition(
      int stream_id, int clock_cycle, int offset, int source_position4,
      int source_position3, int source_position2, int source_position1) = 0;

  virtual void setAXItoBufferChunk(int stream_id, int clock_cycle, int offset,
                                   int target_chunk4, int target_chunk3,
                                   int target_chunk2, int target_chunk1) = 0;
  virtual void setAXItoBufferSourcePosition(int stream_id, int clock_cycle,
                                            int offset, int source_position4,
                                            int source_position3,
                                            int source_position2,
                                            int source_position1) = 0;

  virtual void setInterfaceToBufferChunk(int stream_id, int clock_cycle,
                                         int offset, int target_chunk4,
                                         int target_chunk3, int target_chunk2,
                                         int target_chunk1) = 0;
  virtual void setInterfaceToBufferSourcePosition(
      int stream_id, int clock_cycle, int offset, int source_position4,
      int source_position3, int source_position2, int source_position1) = 0;

  virtual void setBufferToAXIChunk(int stream_id, int clock_cycle, int offset,
                                   int source_chunk4, int source_chunk3,
                                   int source_chunk2, int source_chunk1) = 0;
  virtual void setBufferToAXISourcePosition(int stream_id, int clock_cycle,
                                            int offset, int source_position4,
                                            int source_position3,
                                            int source_position2,
                                            int source_position1) = 0;
};