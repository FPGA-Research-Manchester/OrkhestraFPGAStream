#pragma once
#include <cstdint>
#include "AccelerationModule.hpp"
#include "DMAInterface.hpp"
class DMA : public AccelerationModule, public DMAInterface
{
public:
	~DMA();
	DMA(int* volatile ctrlAXIbaseAddress);

	void setInputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd);
	uint32_t getInputControllerParams(int streamID);
	void setInputControllerStreamAddress(int streamID, uintptr_t address);
	uintptr_t getInputControllerStreamAddress(int streamID);
	void setInputControllerStreamSize(int streamID, int size);
	uint32_t getInputControllerStreamSize(int streamID);
	void startInputController(bool streamActive[16]);
	bool isInputControllerFinished();

	void setRecordSize(int streamID, int recordSize);
	void setRecordChunkIDs(int streamID, int interfaceCycle, int chunkID);

	void setOutputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd);
	uint32_t getOutputControllerParams(int streamID);
	void setOutputControllerStreamAddress(int streamID, uintptr_t address);
	uintptr_t getOutputControllerStreamAddress(int streamID);
	void setOutputControllerStreamSize(int streamID, int size);
	uint32_t getOutputControllerStreamSize(int streamID);
	void startOutputController(bool streamActive[16]);
	bool isOutputControllerFinished();

	void setBufferToInterfaceChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1);
	void setBufferToInterfaceSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1);

	void setAXItoBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1);
	void setAXItoBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1);

	void setInterfaceToBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1);
	void setInterfaceToBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1);

	void setBufferToAXIChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1);
	void setBufferToAXISourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1);
};

