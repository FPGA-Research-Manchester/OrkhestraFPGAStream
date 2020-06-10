#include <cstdio>
#include "DMA.h"
#include <iostream>
#include <vector>
#include <math.h>
#include "rapidcsv.h"
#include <queue> 

/*
Filter: (price < 12000)
1000 rows
 Column |         Type          |
--------+-----------------------+
 id     | bigint                |
 make   | character varying(32) |
 model  | character varying(32) |
 price  | integer               |
*/

struct stringAsciiValue
{
	std::vector<int> values;
	stringAsciiValue() :values(32 / 4, 0) {}
};

void ConvToInt(const std::string& pStr, stringAsciiValue& pVal)
{
	for (int i = 0; i < pStr.length(); i++) {
		pVal.values[i / 4] += int(pStr[i]) << (3 - (i % 4)) * 8;
	}
}

int main()
{
	// Figure out some legit way to get this data type information. For all streams. Would be nice to have this info in structs or sth like that to capture dataType
	std::vector<int> dataTypeSizes;
	dataTypeSizes.push_back(1);
	dataTypeSizes.push_back(8);
	dataTypeSizes.push_back(8);
	dataTypeSizes.push_back(1);
	int recordSize = 0;
	for (auto i = dataTypeSizes.begin(); i != dataTypeSizes.end(); ++i) {
		recordSize += *i;
	}

	// The data probably doesn't come in a csv but it'll do for now
	rapidcsv::Document doc("MOCK_DATA.csv", rapidcsv::LabelParams(-1, -1));
	int recordCount = doc.GetRowCount();

	// Create contiguous data array
	std::vector<int> dbData(doc.GetRowCount() * recordSize);
	int currentLocalDataAddress = 0;
	for (int rowNumber = 0; rowNumber < recordCount; rowNumber++) {
		for (int colNumber = 0; colNumber < 4; colNumber++) {
			switch (colNumber) {
			case 0:
			case 3:
				dbData[currentLocalDataAddress++] = doc.GetCell<int>(colNumber, rowNumber);
				break;
			case 1:
			case 2:
				stringAsciiValue asciiValue = doc.GetCell<stringAsciiValue>(colNumber, rowNumber, ConvToInt);
				for (int i = 0; i < asciiValue.values.size(); i++) {
					dbData[currentLocalDataAddress++] = asciiValue.values[i];
				}
				break;
			}
		}
	}

	// Create the controller memory area //Also hardcode the address to 0xA0000000 we're going to use baremetal
	int* volatile memoryPointer = new int[1048576];
	for (int i = 0; i < 1048576; i++) {
		memoryPointer[i] = -1;
	}
	DMA dmaEngine(memoryPointer);
	// Calculate the controller parameter values based on input data and datatypes
	// Every size metric is 1 integer = 4 bytes = 32 bits
	const int maxDDRBurstSize = 512;
	const int maxChunkSize = 16;
	const int maxDDRSizePerCycle = 4;

	//Input
	int inputStreamID = 0;

	int chunksPerRecord = (recordSize + maxChunkSize - 1) / maxChunkSize; //ceil
	std::cout << "chunksPerRecord:" << chunksPerRecord << std::endl;

	int recordsPerMaxBurstSize = maxDDRBurstSize / recordSize;
	int recordsPerDDRBurst = pow(2, (int)log2(recordsPerMaxBurstSize));
	std::cout << "recordsPerDDRBurst:" << recordsPerDDRBurst << std::endl;

	int DDRBurstLength = ((recordSize * recordsPerDDRBurst) + maxDDRSizePerCycle - 1) / maxDDRSizePerCycle; //ceil (recordSize * recordsPerDDRBurst)/maxDDRSizePerCycle
	std::cout << "DDRBurstLength:" << DDRBurstLength << std::endl;

	// Test different variants later
	int bufferStart = 0;
	int bufferEnd = 15;

	// Set input controller params
	dmaEngine.setInputControllerParams(inputStreamID, DDRBurstLength, recordsPerDDRBurst, bufferStart, bufferEnd);
	dmaEngine.setInputControllerStreamAddress(inputStreamID, reinterpret_cast<uintptr_t>(&dbData[0]));
	dmaEngine.setInputControllerStreamSize(inputStreamID, recordCount);
	dmaEngine.setRecordSize(inputStreamID, chunksPerRecord);
	for (int i = 0; i < chunksPerRecord; i++) {
		dmaEngine.setRecordChunkIDs(inputStreamID, i, i);
	}

	// Output
	int outputStreamID = 1;//Could be 0
	recordCount = 0;
	std::vector<int> outputData(doc.GetRowCount() * recordSize);

	chunksPerRecord = (recordSize + maxChunkSize - 1) / maxChunkSize; //ceil
	std::cout << "chunksPerRecord:" << chunksPerRecord << std::endl;

	recordsPerMaxBurstSize = maxDDRBurstSize / recordSize;
	recordsPerDDRBurst = pow(2, (int)log2(recordsPerMaxBurstSize));
	std::cout << "recordsPerDDRBurst:" << recordsPerDDRBurst << std::endl;

	DDRBurstLength = ((recordSize * recordsPerDDRBurst) + maxDDRSizePerCycle - 1) / maxDDRSizePerCycle;
	std::cout << "DDRBurstLength:" << DDRBurstLength << std::endl;

	bufferStart = 0;
	bufferEnd = 15;

	// Set output controller params
	dmaEngine.setOutputControllerParams(outputStreamID, DDRBurstLength, recordsPerDDRBurst, bufferStart, bufferEnd);
	dmaEngine.setOutputControllerStreamAddress(outputStreamID, reinterpret_cast<uintptr_t>(&outputData[0]));
	dmaEngine.setOutputControllerStreamSize(outputStreamID, recordCount);
	dmaEngine.setRecordSize(outputStreamID, chunksPerRecord);
	for (int i = 0; i < chunksPerRecord; i++) {
		dmaEngine.setRecordChunkIDs(outputStreamID, i, i);
	}

	//dmaEngine.setAXItoBufferChunk(inputStreamID,0,0,0,0,0,0);
	//dmaEngine.setAXItoBufferSourcePosition(inputStreamID, 0, 0, 0, 1, 2, 3);

	//dmaEngine.setBufferToInterfaceChunk(inputStreamID);
	//dmaEngine.setBufferToInterfaceSourcePosition(inputStreamID);

	//dmaEngine.setInterfaceToBufferChunk(outputStreamID);
	//dmaEngine.setInterfaceToBufferSourcePosition(outputStreamID);

	//dmaEngine.setBufferToAXIChunk(outputStreamID);
	//dmaEngine.setBufferToAXISourcePosition(outputStreamID);


	/*
	struct {
		int data[16];
		int chunkID;
	} RecordOnInterface[32];
	*/

	/*
	struct {
		int data[16];
	} Buffer_block[32];
	*/

	/*
	DDRBurstLength = 72
	numRecordsPerBurst = 16;
	maxChunksPerRecord = 2
	chunksPerRecord = 2
	streamID = 0;
	recordSize = 18;
	recordsPerDDRBurst = 16

	/*
	int tarbetChunk[DDRBurstLength][16];
	int sourcePosition[DDRBurstLength][16];
	for(int clockCycle=0; clockCycle < DDRBurstLength; clockCycle++){
		for(int dataPos=0; dataPos<16; dataPos++){
			int targetChunkTemp = targetChunk[clockCycle][dataPos];
			int sourcePositionTemp = sourcePosition[clockCycle][dataPos];
			Buffer_block[targetChunkTemp].data[dataPos]=AXI_Burst[clockCycle].data[sourcePositionTemp];
		}
	}
	*/

	/*

	One for loop for 72 cycles
	First 4 data positions we want to keep
	The rest of the 12 we don't wan't to duplicate anything so we discard

	*/
	int targetBufferChunk = 0;
	const int DONTCARECHUNK = 31;
	const int DONTCAREPOSITION = 15;
	for (int currentClockCycle = 0; currentClockCycle < DDRBurstLength; currentClockCycle++) {
		for (int currentOffset = 0; currentOffset < 4; currentOffset++) {
			if (currentOffset == 0) {
				dmaEngine.setAXItoBufferChunk(inputStreamID, currentClockCycle, currentOffset, targetBufferChunk, targetBufferChunk, targetBufferChunk, targetBufferChunk);
				dmaEngine.setAXItoBufferSourcePosition(inputStreamID, currentClockCycle, currentOffset, (currentClockCycle % 4) * 4 + 3, (currentClockCycle % 4) * 4 + 2, (currentClockCycle % 4) * 4 + 1, (currentClockCycle % 4) * 4 + 0);
			}
			else {
				dmaEngine.setAXItoBufferChunk(inputStreamID, currentClockCycle, currentOffset, DONTCARECHUNK, DONTCARECHUNK, DONTCARECHUNK, DONTCARECHUNK);
				dmaEngine.setAXItoBufferSourcePosition(inputStreamID, currentClockCycle, currentOffset, DONTCAREPOSITION, DONTCAREPOSITION, DONTCAREPOSITION, DONTCAREPOSITION);
			}
		}
		if (currentClockCycle % 4 == 3) {
			targetBufferChunk++;
		}
	}

	/*
	int sourceChunkB2I[32][16];
	int sourcePositionB2I[32][16];
	for(int cyclePosition = maxChunksPerRecord*recordID; cyclePosition < maxChunksPerRecord*recordID + chunksPerRecord; cyclePosition++){
	int dataRead[16];
	for(int dataPos=0; dataPos <16; dataPos++){
		int sourceChunkTemp = sourceChunkB2I[cyclePosition][dataPos];
		dataRead[dataPos] = Buffer_block[sourceChunkTemp].data[dataPos];
	} 
	for(int dataPos = 0; dataPos <16; dataPost++){
		int sourcePositionTemp = sourcePositionB2I[cyclePosition][dataPos];
		RecordOnInterface[cyclePosition].data[dataPos]=dataRead[SourcePositionTemp];
	}
	RecordOnInterface[cyclePosition].chunkID = chunkIDTranslate[cyclePosition];
	}
	*/

	/*
	One cycle is 8x2 chunks  
	*/
	std::queue <int> sourceChunk;
	std::queue <int> targetPosition;
	for (int cycleCounter = 0; cycleCounter < 2; cycleCounter++) {
		for (int cycleStep = 0; cycleStep < 8; cycleStep++) {
			for (int forwardChunkCounter = 0; forwardChunkCounter < cycleStep * 2; forwardChunkCounter++) {
				sourceChunk.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
				targetPosition.push(16 - cycleStep * 2 + forwardChunkCounter);
			}
			for (int currentChunkCounter = cycleStep * 2; currentChunkCounter < 16; currentChunkCounter++) {
				sourceChunk.push((cycleStep + 8 * cycleCounter + cycleCounter));
				targetPosition.push(currentChunkCounter - cycleStep * 2);
			}
			for (int emptyInitialChunkCounter = 0; emptyInitialChunkCounter < cycleStep * 2; emptyInitialChunkCounter++){
				sourceChunk.push(DONTCARECHUNK);
				targetPosition.push(emptyInitialChunkCounter +2);
			}
			sourceChunk.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
			sourceChunk.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
			targetPosition.push(0);
			targetPosition.push(1);
			for (int emptyFinishingChunkCounter = cycleStep * 2 + 2; emptyFinishingChunkCounter < 16; emptyFinishingChunkCounter++) {
				sourceChunk.push(DONTCARECHUNK);
				targetPosition.push(emptyFinishingChunkCounter);
			}
		}
	}

	// For debugging
	for (int i = 0; i < 32; i++) {
		//std::cout << i << ": ";
		for (int j = 0; j < 16; j++) {
			if (!sourceChunk.empty()) {
				//std::cout << sourceChunk.front() << " ";
				//sourceChunk.pop();
				//std::cout << targetPosition.front() << " ";
				//targetPosition.pop();
			}
			else {
				//std::cout << i << j;
			}
		}
		//std::cout << std::endl;
	}

	int input4;
	int input3;
	int input2;
	int input1;
	for (int currentBufferChunk = 0; currentBufferChunk < 32; currentBufferChunk++) {
		for (int currentOffset = 0; currentOffset < 4; currentOffset++) { 
			input1 = sourceChunk.front();
			sourceChunk.pop();
			input2 = sourceChunk.front();
			sourceChunk.pop();
			input3 = sourceChunk.front();
			sourceChunk.pop();
			input4 = sourceChunk.front();
			sourceChunk.pop();
			dmaEngine.setBufferToInterfaceChunk(inputStreamID, currentBufferChunk, currentOffset, input4, input3, input2, input1);
			input1 = targetPosition.front();
			targetPosition.pop();
			input2 = targetPosition.front();
			targetPosition.pop();
			input3 = targetPosition.front();
			targetPosition.pop();
			input4 = targetPosition.front();
			targetPosition.pop();
			dmaEngine.setBufferToInterfaceSourcePosition(inputStreamID, currentBufferChunk, currentOffset, input4, input3, input2, input1);
		}
		std::cout << std::endl;
	}

	// Print out the contents of memory for debugging
	std::cout << std::endl << "Memory contents:" << std::endl;
	for (int i = 0; i < 1048576; i++) {
		if (memoryPointer[i] != -1) {
			std::cout << "Address:" << i << std::endl;
			std::cout << memoryPointer[i] << std::endl;
		}
	}

	delete[] memoryPointer;
	return 0;
}