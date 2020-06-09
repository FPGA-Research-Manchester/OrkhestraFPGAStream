#include <cstdio>
#include "DMA.h"
#include <iostream>
#include <vector>
#include <math.h>
#include "rapidcsv.h"

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
		pVal.values[i / 4] += int(pStr[i]) << (i % 4) * 8;
	}
}

int main()
{
	// Figure out some legit way to get this data type information. For all streams
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

	// Create the controller memory area
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

	int DDRBurstLength = ((recordSize * recordsPerDDRBurst) + maxDDRSizePerCycle - 1) / maxDDRSizePerCycle;
	std::cout << "DDRBurstLength:" << DDRBurstLength << std::endl;

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
	int outputStreamID = 1;
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


	// Then burn pointless cycles between AXI and Buffer 4 times.

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
	streamID = 0;
	recordSize = 18;

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
	int sourceChunkB2I[32][16];
	int sourcePositionB2I[32][16];
	for(int cyclePosition = maxChunksPerRecord*recordID; cyclePosition < maxChunksPerRecord*recordID + recordSize; cyclePosition++){
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
	For AXI2B
	We can have first 32-18=14 cycles be wasted
	and then 72 * 4 = 288 = 18 * 16 which are the rest of the cycles have it be put into the buffers sequentially.
	*/

	const int DONTCARE = 0;
	for (int currentBufferChunk = 0; currentBufferChunk < 18; currentBufferChunk++) {
		for (int currentOffset = 0; currentOffset < 4; currentOffset++) {
			dmaEngine.setAXItoBufferChunk(inputStreamID, currentBufferChunk, currentOffset, DONTCARE, DONTCARE, DONTCARE, DONTCARE);
			dmaEngine.setAXItoBufferSourcePosition(inputStreamID, currentBufferChunk, currentOffset, DONTCARE, DONTCARE, DONTCARE, DONTCARE);
		}
	}
	int targetBufferChunk = 0;
	for (int currentBufferChunk = 18; currentBufferChunk < 32; currentBufferChunk++) {
		for (int currentOffset = 0; currentOffset < 4; currentOffset++) {
			dmaEngine.setAXItoBufferChunk(inputStreamID, currentBufferChunk, currentOffset, targetBufferChunk, targetBufferChunk, targetBufferChunk, targetBufferChunk);
			dmaEngine.setAXItoBufferSourcePosition(inputStreamID, currentBufferChunk, currentOffset, currentOffset * 4 + 3, currentOffset * 4 + 2, currentOffset * 4 + 1, currentOffset * 4 + 0);
		}
		targetBufferChunk++;
	}
	
	/*
	For B2I
	TODO
	*/

























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