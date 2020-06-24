#include "DMA.hpp"
#include <iostream>
#include <math.h>

/*
This is the width of the address for the memory mapped register space in every module.
This parameter's value has to match the parameter in the implemented DMA module.
*/
#define MODULE_ADDRESS_BITS 20

DMA::DMA(int* volatile ctrlAXIbaseAddress) : contollerModulePosition(0)
{
	controlAXIbaseAddress = ctrlAXIbaseAddress;
}

int* volatile DMA::calculateMemoryMappedAddress(int* volatile controlAXIbaseAddress, uint32_t modulePosition, uint32_t moduleInternalAddress) {
	int* volatile returnAddress = controlAXIbaseAddress;
	returnAddress += modulePosition * (1 << MODULE_ADDRESS_BITS); // calculate the main address of the target module
	returnAddress += moduleInternalAddress;
	return returnAddress;
}

void DMA::writeToModule(int* volatile controlAXIbaseAddress, // base address of AXI Control port for PR region
	uint32_t modulePosition, // Position of the module within the PR region (Starting from 1)
	uint32_t moduleInternalAddress, // Internal address of the memory mapped register of the module
	uint32_t writeData		// Data to be written to module's register
) {
	int* volatile registerAddress = calculateMemoryMappedAddress(controlAXIbaseAddress, modulePosition, moduleInternalAddress);
	*registerAddress = writeData;
}
uint32_t DMA::readFromModule(int* volatile controlAXIbaseAddress, // base address of AXI Control port for PR region
	uint32_t modulePosition, // Position of the module within the PR region (Starting from 1)
	uint32_t moduleInternalAddress // Internal address of the memory mapped register of the module
) {
	volatile int* registerAddress = calculateMemoryMappedAddress(controlAXIbaseAddress, modulePosition, moduleInternalAddress);
	uint32_t readData = *registerAddress;
	return readData;
}

//Input Controller
void DMA::setInputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd) {
	writeToModule(controlAXIbaseAddress, contollerModulePosition, (streamID * 4), ((DDRburstSize-1) << 24) + ((int)log2(recordsPerDDRBurst) << 16) + (bufferStart << 8) + (bufferEnd));
}
uint32_t DMA::getInputControllerParams(int streamID) {
	return readFromModule(controlAXIbaseAddress, contollerModulePosition, (streamID * 4));
}
void DMA::setInputControllerStreamAddress(int streamID, uintptr_t address) {
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 6) + (streamID * 4)), address >> 4);
}
uintptr_t DMA::getInputControllerStreamAddress(int streamID) {
	return readFromModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 6) + (streamID * 4)));
}
void DMA::setInputControllerStreamSize(int streamID, int size) {// starting size of stream in amount of records
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((2 << 6) + (streamID * 4)), size);
}
uint32_t DMA::getInputControllerStreamSize(int streamID) {
	return readFromModule(controlAXIbaseAddress, contollerModulePosition, ((2 << 6) + (streamID * 4)));
}
void DMA::startInputController(bool streamActive[16]) {// indicate which streams can be read from DDR and start processing
	int activeStreams = 0;
	for (int i = 15; i >= 0; i--) {
		activeStreams = activeStreams << 1;
		if (streamActive[i])activeStreams = activeStreams + 1;
	}
	writeToModule(controlAXIbaseAddress, contollerModulePosition, (3 << 6), activeStreams);
}
bool DMA::isInputControllerFinished() { // true if all input streams were read from DDR
	uint32_t activeStreams = readFromModule(controlAXIbaseAddress, contollerModulePosition, (3 << 6));
	return (activeStreams == 0);
}
//Input Controller in Crossbar

// How many chunks is a record on a particular streamID
void DMA::setRecordSize(int streamID, int recordSize) {
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 17) + (1 << 9) + (streamID * 4)), recordSize - 1);
}
//set ChunkID at clock cycle of interfaceCycle for records on a particular streamID
void DMA::setRecordChunkIDs(int streamID, int interfaceCycle, int chunkID) {
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 17) + (1 << 13) + (streamID << 8) + (interfaceCycle << 2)), chunkID);
}

//Output Controller
void DMA::setOutputControllerParams(int streamID, int DDRburstSize, int recordsPerDDRBurst, int bufferStart, int bufferEnd) {
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 16) + (streamID * 4)), ((DDRburstSize-1) << 24) + ((int)log2(recordsPerDDRBurst) << 16) + (bufferStart << 8) + (bufferEnd));
}
uint32_t DMA::getOutputControllerParams(int streamID) {
	return readFromModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 16) + (streamID * 4)));
}
void DMA::setOutputControllerStreamAddress(int streamID, uintptr_t address) {
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 16) + (1 << 6) + (streamID * 4)), address >> 4);
}
uintptr_t DMA::getOutputControllerStreamAddress(int streamID) {
	return readFromModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 16) + (1 << 6) + streamID * 4));
}
void DMA::setOutputControllerStreamSize(int streamID, int size) {// starting size of stream in amount of records
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 16) + (2 << 6) + (streamID * 4)), size);
}
uint32_t DMA::getOutputControllerStreamSize(int streamID) {// starting size of stream in amount of records
	return readFromModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 16) + (2 << 6) + (streamID * 4)));
}
void DMA::startOutputController(bool streamActive[16]) {// indicate which streams can be written to DDR and start processing
	int activeStreams = 0;
	for (int i = 15; i >= 0; i--) {
		activeStreams = activeStreams << 1;
		if (streamActive[i])activeStreams = activeStreams + 1;
	}
	writeToModule(controlAXIbaseAddress, 0, ((1 << 16) + (3 << 6)), activeStreams);
}
bool DMA::isOutputControllerFinished() { // true if all streams saw EOS from PR modules
	uint32_t activeStreams = readFromModule(controlAXIbaseAddress, contollerModulePosition, ((1 << 16) + (3 << 6)));
	return (activeStreams == 0);
}

// Input Crossbar from Buffers to Interface
void DMA::setBufferToInterfaceChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1) {
	/*When 32-bit data packets inside the {clockCycle} clock cycle of a record sent to PR Interface of stream with ID {streamID} is read from the BRAM buffers in positions  {offset*4}-{offset*4+3},
	they can read from any source chunk to enable data reordering and duplication. sourceChunk1 represents position {offset*4} etc.*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((2 << 17) + (1 << 16) + (streamID << 12) + (clockCycle << 5) + (offset << 2)), ((sourceChunk4 << 24) + (sourceChunk3 << 16) + (sourceChunk2 << 8) + sourceChunk1));
}
void DMA::setBufferToInterfaceSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) {
	/*When 32-bit data packets inside the {clockCycle} clock cycle of a record sent to PR Interface of stream with ID {streamID} is sent to PR at 32-bit data positions {offset*4}-{offset*4+3},
	they can originate from from any source 32-bit BRAM to enable data reordering and duplication. sourceChunk1 represents position {offset*4} etc..*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((2 << 17) + (streamID << 12) + (clockCycle << 5) + (offset << 2)), ((sourcePosition4 << 24) + (sourcePosition3 << 16) + (sourcePosition2 << 8) + sourcePosition1));
}

// Input Crossbar from AXI/DDR Input to Buffers
void DMA::setAXItoBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1) {
	/*When 32-bit data packets inside the {clockCycle} clock cycle of the AXI read data burst of stream with ID {streamID} reach the BRAM buffers in positions {offset*4}-{offset*4+3},
	they can be written to any target chunk to aid data reordering and duplication.
	 sourceChunk1 represents position {offset*4} etc.*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((2 << 18) + (1 << 17) + (streamID << 13) + (clockCycle << 5) + (offset << 2)), ((targetChunk4 << 24) + (targetChunk3 << 16) + (targetChunk2 << 8) + targetChunk1));
}
void DMA::setAXItoBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) {
	/*When an AXI read data enters the DMA, the {offset*4}-{offset*4+3} data positions for the buffer in the {clockCycle} clock cycle of the AXI transaction of stream with ID {streamID},
	they can select any 32-bit data source from AXI datapath.
	 sourceChunk1 represents position {offset*4} etc..*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((2 << 18) + (streamID << 13) + (clockCycle << 5) + (offset << 2)), ((sourcePosition4 << 24) + (sourcePosition3 << 16) + (sourcePosition2 << 8) + sourcePosition1));
}

// Output Crossbar from Interface to Buffers
void DMA::setInterfaceToBufferChunk(int streamID, int clockCycle, int offset, int targetChunk4, int targetChunk3, int targetChunk2, int targetChunk1) {
	/*When 32-bit data packets inside the {clockCycle} clock cycle of a record coming from the end of the PR Interface of stream with ID {streamID} is written to the BRAM buffers in positions  {offset*4}-{offset*4+3},
	they can be written to any target chunk to enable data reordering/removal of duplication before writing back to DDR.
	 sourceChunk1 represents position {offset*4} etc.*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((3 << 17) + (1 << 16) + (streamID << 12) + (clockCycle << 5) + (offset << 2)), ((targetChunk4 << 24) + (targetChunk3 << 16) + (targetChunk2 << 8) + targetChunk1));
}
void DMA::setInterfaceToBufferSourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) {
	/*When 32-bit data packets inside the {clockCycle} clock cycle of a record coming from PR Interface of stream with ID {streamID} is written to BRAM buffers at 32-bit data positions {offset*4}-{offset*4+3},
	they can originate from from any source 32-bit word from the interface to enable data reordering and duplication.
	sourceChunk1 represents position {offset*4} etc..*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((3 << 17) + (streamID << 12) + (clockCycle << 5) + (offset << 2)), ((sourcePosition4 << 24) + (sourcePosition3 << 16) + (sourcePosition2 << 8) + sourcePosition1));
}

// Output Crossbar from Buffers to AXI/DDR
void DMA::setBufferToAXIChunk(int streamID, int clockCycle, int offset, int sourceChunk4, int sourceChunk3, int sourceChunk2, int sourceChunk1) {
	/*32-bit data words are read from the BRAM buffers in positions {offset*4}-{offset*4+3} to be used inside the {clockCycle} clock cycle of the AXI read data burst of stream with ID {streamID}.
	Every BRAM can read on a different arbitrary position defined by this register.
	 sourceChunk1 represents position {offset*4} etc.*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((3 << 18) + (1 << 17) + (streamID << 13) + (clockCycle << 5) + (offset << 2)), ((sourceChunk4 << 24) + (sourceChunk3 << 16) + (sourceChunk2 << 8) + sourceChunk1));
}
void DMA::setBufferToAXISourcePosition(int streamID, int clockCycle, int offset, int sourcePosition4, int sourcePosition3, int sourcePosition2, int sourcePosition1) {
	/*Routing information for 32-bit data packets {offset*4}-{offset*4+3} inside the {clockCycle} clock cycle of the AXI write data burst of stream with ID {streamID}.
	32-bit data packets {offset*4}-{offset*4+3} are routed to the DDR AXI from any of the BRAM buffers in the middle between the two crossbars.
	sourceChunk1 represents position {offset*4} etc..*/
	writeToModule(controlAXIbaseAddress, contollerModulePosition, ((3 << 18) + (streamID << 13) + (clockCycle << 5) + (offset << 2)), ((sourcePosition4 << 24) + (sourcePosition3 << 16) + (sourcePosition2 << 8) + sourcePosition1));
}
