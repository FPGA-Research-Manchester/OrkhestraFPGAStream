#pragma once
#include "DMAInterface.hpp"
#include "gmock/gmock.h"

class MockDMA : public DMAInterface {
 public:
  MOCK_METHOD(void, SetInputControllerParams,
              (int streamID, int DDRburstSize, int recordsPerDDRBurst,
               int bufferStart, int bufferEnd),
              (override));
  MOCK_METHOD(uint32_t, GetInputControllerParams, (int streamID), (override));
  MOCK_METHOD(void, SetInputControllerStreamAddress,
              (int streamID, uintptr_t address), (override));
  MOCK_METHOD(uintptr_t, GetInputControllerStreamAddress, (int streamID),
              (override));
  MOCK_METHOD(void, SetInputControllerStreamSize, (int streamID, int size),
              (override));
  MOCK_METHOD(uint32_t, GetInputControllerStreamSize, (int streamID),
              (override));
  MOCK_METHOD(void, StartInputController, (bool streamActive[16]), (override));
  MOCK_METHOD(bool, IsInputControllerFinished, (), (override));

  MOCK_METHOD(void, SetRecordSize, (int streamID, int recordSize), (override));
  MOCK_METHOD(void, SetRecordChunkIDs,
              (int streamID, int interfaceCycle, int chunkID), (override));

  MOCK_METHOD(void, SetOutputControllerParams,
              (int streamID, int DDRburstSize, int recordsPerDDRBurst,
               int bufferStart, int bufferEnd),
              (override));
  MOCK_METHOD(uint32_t, GetOutputControllerParams, (int streamID), (override));
  MOCK_METHOD(void, SetOutputControllerStreamAddress,
              (int streamID, uintptr_t address), (override));
  MOCK_METHOD(uintptr_t, GetOutputControllerStreamAddress, (int streamID),
              (override));
  MOCK_METHOD(void, SetOutputControllerStreamSize, (int streamID, int size),
              (override));
  MOCK_METHOD(uint32_t, GetOutputControllerStreamSize, (int streamID),
              (override));
  MOCK_METHOD(void, StartOutputController, (bool streamActive[16]), (override));
  MOCK_METHOD(bool, IsOutputControllerFinished, (), (override));

  MOCK_METHOD(void, SetBufferToInterfaceChunk,
              (int streamID, int clockCycle, int offset, int sourceChunk4,
               int sourceChunk3, int sourceChunk2, int sourceChunk1),
              (override));
  MOCK_METHOD(void, SetBufferToInterfaceSourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));

  MOCK_METHOD(void, SetAXItoBufferChunk,
              (int streamID, int clockCycle, int offset, int targetChunk4,
               int targetChunk3, int targetChunk2, int targetChunk1),
              (override));
  MOCK_METHOD(void, SetAXItoBufferSourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));

  MOCK_METHOD(void, SetInterfaceToBufferChunk,
              (int streamID, int clockCycle, int offset, int targetChunk4,
               int targetChunk3, int targetChunk2, int targetChunk1),
              (override));
  MOCK_METHOD(void, SetInterfaceToBufferSourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));

  MOCK_METHOD(void, SetBufferToAXIChunk,
              (int streamID, int clockCycle, int offset, int sourceChunk4,
               int sourceChunk3, int sourceChunk2, int sourceChunk1),
              (override));
  MOCK_METHOD(void, SetBufferToAXISourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));
};