#include <cstdio>
#include <iostream>
#include <vector>
#include <math.h>
#include <rapidcsv.h>
#include <queue> 
#include "DMA.hpp"
#include "Filter.hpp"

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

struct StringAsciiValue
{
	std::vector<int> values;
	StringAsciiValue() :values(32 / 4, 0) {}
};

void ConvToInt(const std::string& pStr, StringAsciiValue& pVal)
{
	for (int i = 0; i < pStr.length(); i++) {
		pVal.values[i / 4] += int(pStr[i]) << (3 - (i % 4)) * 8;
	}
}

void FillDataArray(std::vector<int>& dbData, rapidcsv::Document* dbDataFile){
	int currentLocalDataAddress = 0;
	for (int rowNumber = 0; rowNumber < dbDataFile->GetRowCount(); rowNumber++) {
		for (int colNumber = 0; colNumber < 4; colNumber++) {
			switch (colNumber) {
			case 0:
			case 3:
				dbData[currentLocalDataAddress++] = dbDataFile->GetCell<int>(colNumber, rowNumber);
				break;
			case 1:
			case 2:
				StringAsciiValue asciiValue = dbDataFile->GetCell<StringAsciiValue>(colNumber, rowNumber, ConvToInt);
				for (int i = 0; i < asciiValue.values.size(); i++) {
					dbData[currentLocalDataAddress++] = asciiValue.values[i];
				}
				break;
			}
		}
	}
}

//TODO: Get rid of magic numbers
struct DMACrossbarSetupData {
	DMACrossbarSetupData() : chunkData(16), positionData(16) {}
	// TODO: Think about these names!
	std::vector<int> chunkData;
	std::vector<int> positionData;
};

struct DMASetupData {
	DMASetupData() : crossbarSetupData(32) {}
	int streamID;
	int DDRBurstLength;
	int recordsPerDDRBurst;
	uintptr_t streamAddress;
	int recordCount;
	int chunksPerRecord;
	int bufferStart;
	int bufferEnd;
	std::vector<std::tuple<int, int>> recordChunkIDs;
	std::vector<DMACrossbarSetupData> crossbarSetupData;
	bool isInputStream;
};

void CalculateDMAStreamSetupData(DMASetupData& streamSetupData, const int& maxChunkSize, const int& maxDDRBurstSize, const int& maxDDRSizePerCycle, std::vector<int>& dbData, int recordSize)
{
	streamSetupData.chunksPerRecord = (recordSize + maxChunkSize - 1) / maxChunkSize; //ceil

	// Temporarily for now.
	for (int i = 0; i < streamSetupData.chunksPerRecord; i++) {
		streamSetupData.recordChunkIDs.push_back(std::make_tuple(i,i));
	}

	int recordsPerMaxBurstSize = maxDDRBurstSize / recordSize;
	streamSetupData.recordsPerDDRBurst = pow(2, (int)log2(recordsPerMaxBurstSize));

	streamSetupData.DDRBurstLength = ((recordSize * streamSetupData.recordsPerDDRBurst) + maxDDRSizePerCycle - 1) / maxDDRSizePerCycle; //ceil (recordSize * recordsPerDDRBurst) / maxDDRSizePerCycle

	// Temporarily for now
	streamSetupData.bufferStart = 0;
	streamSetupData.bufferEnd = 15;

	streamSetupData.streamAddress = reinterpret_cast<uintptr_t>(&dbData[0]);
}

void SetUpDMAIOStreams(DMASetupData& streamSetupData, DMA& dmaEngine)
{
	if (streamSetupData.isInputStream)
	{
		dmaEngine.setInputControllerParams(streamSetupData.streamID, streamSetupData.DDRBurstLength, streamSetupData.recordsPerDDRBurst, streamSetupData.bufferStart, streamSetupData.bufferEnd);
		dmaEngine.setInputControllerStreamAddress(streamSetupData.streamID, streamSetupData.streamAddress);
		dmaEngine.setInputControllerStreamSize(streamSetupData.streamID, streamSetupData.recordCount);
	}
	else
	{
		dmaEngine.setOutputControllerParams(streamSetupData.streamID, streamSetupData.DDRBurstLength, streamSetupData.recordsPerDDRBurst, streamSetupData.bufferStart, streamSetupData.bufferEnd);
		dmaEngine.setOutputControllerStreamAddress(streamSetupData.streamID, streamSetupData.streamAddress);
		dmaEngine.setOutputControllerStreamSize(streamSetupData.streamID, streamSetupData.recordCount);
	}
	dmaEngine.setRecordSize(streamSetupData.streamID, streamSetupData.chunksPerRecord);
	for (auto& chunkIDPair : streamSetupData.recordChunkIDs)
	{
		dmaEngine.setRecordChunkIDs(streamSetupData.streamID, std::get<0>(chunkIDPair), std::get<1>(chunkIDPair));
	}
}

// TODO: Change to better names
void SetCrossbarSetupDataForStream(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, DMASetupData& streamSetupData)
{
	for (int currentBufferChunk = 0; currentBufferChunk < 32; currentBufferChunk++) {
		DMACrossbarSetupData currentChunkData;
		for (int currentDataInput = 0; currentDataInput < 16; currentDataInput++) {
			currentChunkData.chunkData[currentDataInput] = sourceChunks.front();
			sourceChunks.pop();
			currentChunkData.positionData[currentDataInput] = targetPositions.front();
			targetPositions.pop();
		}
		streamSetupData.crossbarSetupData[currentBufferChunk] = currentChunkData;
	}
}

void SetUpDMACrossbars(DMASetupData& streamSetupData, DMA& dmaEngine)
{
	for (size_t currentChunkIndex = 0; currentChunkIndex < streamSetupData.crossbarSetupData.size(); ++currentChunkIndex) {
		if (streamSetupData.isInputStream) {
			for (int currentOffset = 0; currentOffset < 4; currentOffset++) {
				dmaEngine.setBufferToInterfaceChunk(streamSetupData.streamID, currentChunkIndex, currentOffset, streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[3 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[2 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[1 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[0 + currentOffset * 4]);
				dmaEngine.setBufferToInterfaceSourcePosition(streamSetupData.streamID, currentChunkIndex, currentOffset, streamSetupData.crossbarSetupData[currentChunkIndex].positionData[3 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].positionData[2 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].positionData[1 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].positionData[0 + currentOffset * 4]);
			}
		}
		else {
			for (int currentOffset = 0; currentOffset < 4; currentOffset++) {
				dmaEngine.setInterfaceToBufferChunk(streamSetupData.streamID, currentChunkIndex, currentOffset, streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[3 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[2 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[1 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].chunkData[0 + currentOffset * 4]);
				dmaEngine.setInterfaceToBufferSourcePosition(streamSetupData.streamID, currentChunkIndex, currentOffset, streamSetupData.crossbarSetupData[currentChunkIndex].positionData[3 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].positionData[2 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].positionData[1 + currentOffset * 4], streamSetupData.crossbarSetupData[currentChunkIndex].positionData[0 + currentOffset * 4]);
			}
		}
	}
}

void WriteSetupDataToDMAModule(std::vector<DMASetupData>& setupDataForDMA, DMA& dmaEngine)
{
	for (auto& streamSetupData : setupDataForDMA)
	{
		SetUpDMAIOStreams(streamSetupData, dmaEngine);
		SetUpDMACrossbars(streamSetupData, dmaEngine);
	}
}

void CalculateBufferToInterfaceSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION)
{
	for (int cycleCounter = 0; cycleCounter < 2; cycleCounter++) {
		for (int cycleStep = 0; cycleStep < 8; cycleStep++) {
			// Initial chunk
			for (int forwardChunkCounter = 0; forwardChunkCounter < cycleStep * 2; forwardChunkCounter++) {
				sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
			}
			for (int currentChunkCounter = cycleStep * 2; currentChunkCounter < 16; currentChunkCounter++) {
				sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter));
				targetPositions.push(15 - currentChunkCounter);
			}
			for (int forwardChunkCounter = 0; forwardChunkCounter < cycleStep * 2; forwardChunkCounter++) {
				targetPositions.push(15 - forwardChunkCounter);
			}
			// Last chunk
			targetPositions.push(15 - cycleStep * 2);
			targetPositions.push(15 - (cycleStep * 2 + 1));
			for (int emptyInitialChunkCounter = 0; emptyInitialChunkCounter < cycleStep * 2; emptyInitialChunkCounter++) {
				sourceChunks.push(ANY_CHUNK);
				targetPositions.push(ANY_POSITION);
			}
			sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
			sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
			for (int emptyFinishingChunkCounter = cycleStep * 2 + 2; emptyFinishingChunkCounter < 16; emptyFinishingChunkCounter++) {
				sourceChunks.push(ANY_CHUNK);
				targetPositions.push(ANY_POSITION);
			}
		}
	}
}

void FindInputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& inputStreamSetupData)
{
	std::queue <int> sourceChunks;
	std::queue <int> targetPositions;
	CalculateBufferToInterfaceSetupConfig(sourceChunks, targetPositions, ANY_CHUNK, ANY_POSITION);
	SetCrossbarSetupDataForStream(sourceChunks, targetPositions, inputStreamSetupData);
}

void CalculateInterfaceToBufferSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION)
{
	for (int cycleCounter = 0; cycleCounter < 2; cycleCounter++) {
		for (int cycleStep = 0; cycleStep < 8; cycleStep++) {
			// Initial chunk
			for (int forwardChunkCounter = 16 - cycleStep * 2; forwardChunkCounter < 16; forwardChunkCounter++) {
				sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
				targetPositions.push(15 - forwardChunkCounter);
			}
			for (int currentChunkCounter = 0; currentChunkCounter < 16 - cycleStep * 2; currentChunkCounter++) {
				sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter));
				targetPositions.push(15 - currentChunkCounter);
			}
			// Last chunk
			sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
			sourceChunks.push((cycleStep + 8 * cycleCounter + cycleCounter) + 1);
			for (int emptyInitialChunkCounter = 0; emptyInitialChunkCounter < cycleStep * 2; emptyInitialChunkCounter++) {
				sourceChunks.push(ANY_CHUNK);
				targetPositions.push(ANY_POSITION);
			}
			targetPositions.push(15 - 0);
			targetPositions.push(15 - 1);
			for (int emptyFinishingChunkCounter = cycleStep * 2 + 2; emptyFinishingChunkCounter < 16; emptyFinishingChunkCounter++) {
				sourceChunks.push(ANY_CHUNK);
				targetPositions.push(ANY_POSITION);
			}
		}
	}
}

void FindOutputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& outputStreamSetupData)
{
	std::queue <int> sourceChunks;
	std::queue <int> targetPositions;
	CalculateInterfaceToBufferSetupConfig(sourceChunks, targetPositions, ANY_CHUNK, ANY_POSITION);
	SetCrossbarSetupDataForStream(sourceChunks, targetPositions, outputStreamSetupData);

}

void SetupDMAModule(rapidcsv::Document& doc, std::vector<int>& dbData, int recordSize, DMA& dmaEngine)
{
	// Calculate the controller parameter values based on input data and datatypes
	// Every size metric is 1 integer = 4 bytes = 32 bits
	const int MAX_DDR_BURST_SIZE = 512;
	const int MAX_CHUNK_SIZE = 16;
	const int MAX_DDR_SIZE_PER_CYCLE = 4;

	//Input
	DMASetupData inputStreamSetupData;
	int inputStreamID = 0;
	inputStreamSetupData.streamID = inputStreamID;
	inputStreamSetupData.isInputStream = true;
	inputStreamSetupData.recordCount = doc.GetRowCount();
	CalculateDMAStreamSetupData(inputStreamSetupData, MAX_CHUNK_SIZE, MAX_DDR_BURST_SIZE, MAX_DDR_SIZE_PER_CYCLE, dbData, recordSize);

	// Output
	DMASetupData outputStreamSetupData;
	int outputStreamID = 1;
	outputStreamSetupData.streamID = outputStreamID;
	outputStreamSetupData.isInputStream = false;
	outputStreamSetupData.recordCount = 0;
	CalculateDMAStreamSetupData(outputStreamSetupData, MAX_CHUNK_SIZE, MAX_DDR_BURST_SIZE, MAX_DDR_SIZE_PER_CYCLE, dbData, recordSize);

	const int ANY_CHUNK = 31;
	const int ANY_POSITION = 3;
	FindInputCrossbarSetupData(ANY_CHUNK, ANY_POSITION, inputStreamSetupData);
	FindOutputCrossbarSetupData(ANY_CHUNK, ANY_POSITION, outputStreamSetupData);
	
	std::vector<DMASetupData> setupDataForDMA;
	setupDataForDMA.push_back(inputStreamSetupData);
	setupDataForDMA.push_back(outputStreamSetupData);
	WriteSetupDataToDMAModule(setupDataForDMA, dmaEngine);
}

void SetupFilterModule(Filter& filterModule)
{
	uint32_t streamIDInput = 0;
	uint32_t streamIDValidOutput = 1;
	uint32_t streamIDInvalidOutput = 0;

	filterModule.filterSetStreamIDs(streamIDInput, streamIDValidOutput, streamIDInvalidOutput);

	bool requestOnInvalidIfLast = true;
	bool forwardInvalidRecordFirstChunk = false;
	bool forwardFullInvalidRecords = false;

	bool firstModuleInResourceElasticChain = true;
	bool lastModuleInResourceElasticChain = true;

	filterModule.filterSetMode(requestOnInvalidIfLast, forwardInvalidRecordFirstChunk, forwardFullInvalidRecords, firstModuleInResourceElasticChain, lastModuleInResourceElasticChain);

	uint32_t chunkID = 1;
	uint32_t dataPosition = 1;

	uint32_t lessThanCompare = 0;
	uint32_t DONTCARECOMPARE = 0;

	filterModule.filterSetCompareTypes(chunkID, dataPosition, lessThanCompare, DONTCARECOMPARE, DONTCARECOMPARE, DONTCARECOMPARE);

	uint32_t compareNumber = 0;
	uint32_t compareReferenceValue = 12000;

	filterModule.filterSetCompareReferenceValue(chunkID, dataPosition, compareNumber + 1, compareReferenceValue);

	uint32_t DNF_Clause_ID = 0;
	uint8_t positiveLiteralType = 1;

	filterModule.filterSetDNFClauseLiteral(DNF_Clause_ID, compareNumber, chunkID, dataPosition, positiveLiteralType);

	uint32_t datapathWidth = 16;

	filterModule.writeDNFClauseLiteralsToFilter_1CMP_8DNF(datapathWidth);
}

int main()
{
	// Figure out some legit way to get this data type information. For all streams. Would be nice to have this info in structs or sth like that to capture dataType
	std::vector<int> dataTypeSizes;
	dataTypeSizes.push_back(1);
	dataTypeSizes.push_back(8);
	dataTypeSizes.push_back(8);
	dataTypeSizes.push_back(1);
	// TODO: Check lambda functions performance later here
	int recordSize = 0;
	for (auto i = dataTypeSizes.begin(); i != dataTypeSizes.end(); ++i) {
		recordSize += *i;
	}

	// The data probably doesn't come in a csv but it'll do for now
	rapidcsv::Document doc("MOCK_DATA.csv", rapidcsv::LabelParams(-1, -1));
	// Create contiguous data array
	std::vector<int> dbData(doc.GetRowCount() * recordSize);
	FillDataArray(dbData, &doc);

	// Create the controller memory area //Also hardcode the address to 0xA0000000 we're going to use baremetal
	int* volatile memoryPointer = new int[2097152];
	for (int i = 0; i < 2097152; i++) {
		memoryPointer[i] = -1;
	}

	DMA dmaEngine(memoryPointer);
	SetupDMAModule(doc, dbData, recordSize, dmaEngine);

	// Setup the filter module
	Filter filterModule(memoryPointer, 1);
	SetupFilterModule(filterModule);

	bool streamActive[16] = { false };
	streamActive[0] = true;
	dmaEngine.startInputController(streamActive);
	dmaEngine.startOutputController(streamActive);

	// Print out the contents of memory for debugging
	//std::cout << std::endl << "Memory contents:" << std::endl;
	//for (int i = 0; i < 1048576; i++) {
	//	if (memoryPointer[i] != -1) {
	//		std::cout << "Address:" << i << std::endl;
	//		std::cout << memoryPointer[i] << std::endl;
	//	}
	//}

	// check isInputControllerFinished and isOutputControllerFinished

	delete[] memoryPointer;
	return 0;
}
