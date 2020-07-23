#pragma once
class DMAInterface
{
public:
	virtual ~DMAInterface(){}

	virtual void setInputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd) = 0;
	virtual uint32_t getInputControllerParams(int streamID) = 0;
	virtual void setInputControllerStreamAddress(int streamID, uintptr_t address) = 0;
	virtual uintptr_t getInputControllerStreamAddress(int streamID) = 0;
	virtual void setInputControllerStreamSize(int streamID, int size) = 0;
	virtual uint32_t getInputControllerStreamSize(int streamID) = 0;
	virtual void startInputController(bool streamActive[16]) = 0;
	virtual bool isInputControllerFinished() = 0;

	virtual void setRecordSize(int streamID, int recordSize) = 0;
	virtual void setRecordChunkIDs(int streamID, int interfaceCycle, int chunkID) = 0;

	virtual void setOutputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd) = 0;
	virtual uint32_t getOutputControllerParams(int streamID) = 0;
	virtual void setOutputControllerStreamAddress(int streamID, uintptr_t address) = 0;
	virtual uintptr_t getOutputControllerStreamAddress(int streamID) = 0;
	virtual void setOutputControllerStreamSize(int streamID, int size) = 0;
	virtual uint32_t getOutputControllerStreamSize(int streamID) = 0;
	virtual void startOutputController(bool streamActive[16]) = 0;
	virtual bool isOutputControllerFinished() = 0;

	virtual void setBufferToInterfaceChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1) = 0;
	virtual void setBufferToInterfaceSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) = 0;

	virtual void setAXItoBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1) = 0;
	virtual void setAXItoBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) = 0;

	virtual void setInterfaceToBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1) = 0;
	virtual void setInterfaceToBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) = 0;

	virtual void setBufferToAXIChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1) = 0;
	virtual void setBufferToAXISourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) = 0;
};