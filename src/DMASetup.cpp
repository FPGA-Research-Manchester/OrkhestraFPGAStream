#include "DMASetup.hpp"
#include <queue>
#include <math.h>
#include <cstdio>
#include <tuple>
#include "DMACrossbarSetup.hpp"

void DMASetup::SetupDMAModule(DMAInterface& dmaEngine, std::vector<int>& dbData, int recordSize, int recordCount, int inputStreamID, int outputStreamID) {
	// Calculate the controller parameter values based on input data and datatypes
	// Every size metric is 1 integer = 4 bytes = 32 bits
	const int MAX_DDR_BURST_SIZE = 512;
	const int MAX_CHUNK_SIZE = 16;
	const int MAX_DDR_SIZE_PER_CYCLE = 4;

	//Input
	DMASetupData inputStreamSetupData;
	inputStreamSetupData.streamID = inputStreamID;
	inputStreamSetupData.isInputStream = true;
	inputStreamSetupData.recordCount = recordCount;
	//inputStreamSetupData.recordCount = doc.GetRowCount();
	CalculateDMAStreamSetupData(inputStreamSetupData, MAX_CHUNK_SIZE, MAX_DDR_BURST_SIZE, MAX_DDR_SIZE_PER_CYCLE, dbData, recordSize);

	// Output
	DMASetupData outputStreamSetupData;
	outputStreamSetupData.streamID = outputStreamID;
	outputStreamSetupData.isInputStream = false;
	outputStreamSetupData.recordCount = 0;
	CalculateDMAStreamSetupData(outputStreamSetupData, MAX_CHUNK_SIZE, MAX_DDR_BURST_SIZE, MAX_DDR_SIZE_PER_CYCLE, dbData, recordSize);

	const int ANY_CHUNK = 31;
	const int ANY_POSITION = 3;
	DMACrossbarSetup crossbarConfigurationFinder;
	crossbarConfigurationFinder.FindInputCrossbarSetupData(ANY_CHUNK, ANY_POSITION, inputStreamSetupData);
	crossbarConfigurationFinder.FindOutputCrossbarSetupData(ANY_CHUNK, ANY_POSITION, outputStreamSetupData);

	std::vector<DMASetupData> setupDataForDMA;
	setupDataForDMA.push_back(inputStreamSetupData);
	setupDataForDMA.push_back(outputStreamSetupData);
	WriteSetupDataToDMAModule(setupDataForDMA, dmaEngine);
}

void DMASetup::WriteSetupDataToDMAModule(std::vector<DMASetupData>& setupDataForDMA, DMAInterface& dmaEngine)
{
	for (auto& streamSetupData : setupDataForDMA)
	{
		SetUpDMAIOStreams(streamSetupData, dmaEngine);
		SetUpDMACrossbars(streamSetupData, dmaEngine);
	}
}

void DMASetup::SetUpDMACrossbars(DMASetupData& streamSetupData, DMAInterface& dmaEngine)
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

void DMASetup::SetUpDMAIOStreams(DMASetupData& streamSetupData, DMAInterface& dmaEngine)
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

void DMASetup::CalculateDMAStreamSetupData(DMASetupData& streamSetupData, const int& maxChunkSize, const int& maxDDRBurstSize, const int& maxDDRSizePerCycle, std::vector<int>& dbData, int recordSize)
{
	streamSetupData.chunksPerRecord = (recordSize + maxChunkSize - 1) / maxChunkSize; //ceil

	// Temporarily for now.
	for (int i = 0; i < streamSetupData.chunksPerRecord; i++) {
		streamSetupData.recordChunkIDs.push_back(std::make_tuple(i, i));
	}

	int recordsPerMaxBurstSize = maxDDRBurstSize / recordSize;
	streamSetupData.recordsPerDDRBurst = pow(2, (int)log2(recordsPerMaxBurstSize));

	streamSetupData.DDRBurstLength = ((recordSize * streamSetupData.recordsPerDDRBurst) + maxDDRSizePerCycle - 1) / maxDDRSizePerCycle; //ceil (recordSize * recordsPerDDRBurst) / maxDDRSizePerCycle

	// Temporarily for now
	streamSetupData.bufferStart = 0;
	streamSetupData.bufferEnd = 15;

	streamSetupData.streamAddress = reinterpret_cast<uintptr_t>(&dbData[0]);
}
