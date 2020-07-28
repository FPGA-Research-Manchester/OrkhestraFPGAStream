#pragma once
#include "DMAInterface.hpp"
#include "gmock/gmock.h"

class MockDMA : public DMAInterface {
 public:
  MOCK_METHOD(void, setInputControllerParams,
              (int streamID, int DDRburstSize, int recordsPerDDRBurst,
               int bufferStart, int bufferEnd),
              (override));
  MOCK_METHOD(uint32_t, getInputControllerParams, (int streamID), (override));
  MOCK_METHOD(void, setInputControllerStreamAddress,
              (int streamID, uintptr_t address), (override));
  MOCK_METHOD(uintptr_t, getInputControllerStreamAddress, (int streamID),
              (override));
  MOCK_METHOD(void, setInputControllerStreamSize, (int streamID, int size),
              (override));
  MOCK_METHOD(uint32_t, getInputControllerStreamSize, (int streamID),
              (override));
  MOCK_METHOD(void, startInputController, (bool streamActive[16]), (override));
  MOCK_METHOD(bool, isInputControllerFinished, (), (override));

  MOCK_METHOD(void, setRecordSize, (int streamID, int recordSize), (override));
  MOCK_METHOD(void, setRecordChunkIDs,
              (int streamID, int interfaceCycle, int chunkID), (override));

  MOCK_METHOD(void, setOutputControllerParams,
              (int streamID, int DDRburstSize, int recordsPerDDRBurst,
               int bufferStart, int bufferEnd),
              (override));
  MOCK_METHOD(uint32_t, getOutputControllerParams, (int streamID), (override));
  MOCK_METHOD(void, setOutputControllerStreamAddress,
              (int streamID, uintptr_t address), (override));
  MOCK_METHOD(uintptr_t, getOutputControllerStreamAddress, (int streamID),
              (override));
  MOCK_METHOD(void, setOutputControllerStreamSize, (int streamID, int size),
              (override));
  MOCK_METHOD(uint32_t, getOutputControllerStreamSize, (int streamID),
              (override));
  MOCK_METHOD(void, startOutputController, (bool streamActive[16]), (override));
  MOCK_METHOD(bool, isOutputControllerFinished, (), (override));

  MOCK_METHOD(void, setBufferToInterfaceChunk,
              (int streamID, int clockCycle, int offset, int sourceChunk4,
               int sourceChunk3, int sourceChunk2, int sourceChunk1),
              (override));
  MOCK_METHOD(void, setBufferToInterfaceSourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));

  MOCK_METHOD(void, setAXItoBufferChunk,
              (int streamID, int clockCycle, int offset, int targetChunk4,
               int targetChunk3, int targetChunk2, int targetChunk1),
              (override));
  MOCK_METHOD(void, setAXItoBufferSourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));

  MOCK_METHOD(void, setInterfaceToBufferChunk,
              (int streamID, int clockCycle, int offset, int targetChunk4,
               int targetChunk3, int targetChunk2, int targetChunk1),
              (override));
  MOCK_METHOD(void, setInterfaceToBufferSourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));

  MOCK_METHOD(void, setBufferToAXIChunk,
              (int streamID, int clockCycle, int offset, int sourceChunk4,
               int sourceChunk3, int sourceChunk2, int sourceChunk1),
              (override));
  MOCK_METHOD(void, setBufferToAXISourcePosition,
              (int streamID, int clockCycle, int offset, int sourcePosition4,
               int sourcePosition3, int sourcePosition2, int sourcePosition1),
              (override));
};