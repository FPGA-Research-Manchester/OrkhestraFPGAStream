#pragma once
#include "AccelerationModule.hpp"
#include "DMAInterface.hpp"
#include <cstdint>
class DMA : public AccelerationModule, public DMAInterface
{
public:
	~DMA() override;
	DMA(int* volatile ctrlAXIbaseAddress);

	void setInputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd) override;
	auto getInputControllerParams(int streamID) -> uint32_t override;
	void setInputControllerStreamAddress(int streamID, uintptr_t address) override;
	auto getInputControllerStreamAddress(int streamID) -> uintptr_t override;
	void setInputControllerStreamSize(int streamID, int size) override;
	auto getInputControllerStreamSize(int streamID) -> uint32_t override;
	void startInputController(bool streamActive[16]) override;
	auto isInputControllerFinished() -> bool override;

	void setRecordSize(int streamID, int recordSize) override;
	void setRecordChunkIDs(int streamID, int interfaceCycle, int chunkID) override;

	void setOutputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd) override;
	auto getOutputControllerParams(int streamID) -> uint32_t override;
	void setOutputControllerStreamAddress(int streamID, uintptr_t address) override;
	auto getOutputControllerStreamAddress(int streamID) -> uintptr_t override;
	void setOutputControllerStreamSize(int streamID, int size) override;
	auto getOutputControllerStreamSize(int streamID) -> uint32_t override;
	void startOutputController(bool streamActive[16]) override;
	auto isOutputControllerFinished() -> bool override;

	void setBufferToInterfaceChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1) override;
	void setBufferToInterfaceSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) override;

	void setAXItoBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1) override;
	void setAXItoBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) override;

	void setInterfaceToBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1) override;
	void setInterfaceToBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) override;

	void setBufferToAXIChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1) override;
	void setBufferToAXISourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) override;
};

