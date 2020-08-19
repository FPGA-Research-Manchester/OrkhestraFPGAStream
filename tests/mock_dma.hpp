#pragma once
#include <cstdint>

#include "dma_interface.hpp"
#include "gmock/gmock.h"

class MockDMA : public DMAInterface {
 public:
  MOCK_METHOD(void, SetInputControllerParams,
              (int stream_id, int ddr_burst_size, int records_per_ddr_burst,
               int buffer_start, int buffer_end),
              (override));
  MOCK_METHOD(volatile int, GetInputControllerParams, (int stream_id),
              (override));
  MOCK_METHOD(void, SetInputControllerStreamAddress,
              (int stream_id, uintptr_t address), (override));
  MOCK_METHOD(volatile uintptr_t, GetInputControllerStreamAddress,
              (int stream_id),
              (override));
  MOCK_METHOD(void, SetInputControllerStreamSize, (int stream_id, int size),
              (override));
  MOCK_METHOD(volatile int, GetInputControllerStreamSize, (int stream_id),
              (override));
  MOCK_METHOD(void, StartInputController, (bool stream_active[16]), (override));
  MOCK_METHOD(bool, IsInputControllerFinished, (), (override));

  MOCK_METHOD(void, SetRecordSize, (int stream_id, int recordSize), (override));
  MOCK_METHOD(void, SetRecordChunkIDs,
              (int stream_id, int interfaceCycle, int chunkID), (override));

  MOCK_METHOD(void, SetOutputControllerParams,
              (int stream_id, int ddr_burst_size, int records_per_ddr_burst,
               int buffer_start, int buffer_end),
              (override));
  MOCK_METHOD(volatile int, GetOutputControllerParams, (int stream_id),
              (override));
  MOCK_METHOD(void, SetOutputControllerStreamAddress,
              (int stream_id, uintptr_t address), (override));
  MOCK_METHOD(volatile uintptr_t, GetOutputControllerStreamAddress,
              (int stream_id),
              (override));
  MOCK_METHOD(void, SetOutputControllerStreamSize, (int stream_id, int size),
              (override));
  MOCK_METHOD(volatile int, GetOutputControllerStreamSize, (int stream_id),
              (override));
  MOCK_METHOD(void, StartOutputController, (bool stream_active[16]),
              (override));
  MOCK_METHOD(bool, IsOutputControllerFinished, (), (override));

  MOCK_METHOD(void, SetBufferToInterfaceChunk,
              (int stream_id, int clockCycle, int offset, int sourceChunk4,
               int sourceChunk3, int sourceChunk2, int sourceChunk1),
              (override));
  MOCK_METHOD(void, SetBufferToInterfaceSourcePosition,
              (int stream_id, int clockCycle, int offset, int source_position4,
               int source_position3, int source_position2,
               int source_position1),
              (override));

  MOCK_METHOD(void, SetAXItoBufferChunk,
              (int stream_id, int clockCycle, int offset, int target_chunk4,
               int target_chunk3, int target_chunk2, int target_chunk1),
              (override));
  MOCK_METHOD(void, SetAXItoBufferSourcePosition,
              (int stream_id, int clockCycle, int offset, int source_position4,
               int source_position3, int source_position2,
               int source_position1),
              (override));

  MOCK_METHOD(void, SetInterfaceToBufferChunk,
              (int stream_id, int clockCycle, int offset, int target_chunk4,
               int target_chunk3, int target_chunk2, int target_chunk1),
              (override));
  MOCK_METHOD(void, SetInterfaceToBufferSourcePosition,
              (int stream_id, int clockCycle, int offset, int source_position4,
               int source_position3, int source_position2,
               int source_position1),
              (override));

  MOCK_METHOD(void, SetBufferToAXIChunk,
              (int stream_id, int clockCycle, int offset, int sourceChunk4,
               int sourceChunk3, int sourceChunk2, int sourceChunk1),
              (override));
  MOCK_METHOD(void, SetBufferToAXISourcePosition,
              (int stream_id, int clockCycle, int offset, int source_position4,
               int source_position3, int source_position2,
               int source_position1),
              (override));
};