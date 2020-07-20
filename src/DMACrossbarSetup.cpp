#include "DMACrossbarSetup.hpp"
#include "DMACrossbarSetupData.hpp"

void DMACrossbarSetup::FindOutputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& outputStreamSetupData)
{
	std::queue <int> sourceChunks;
	std::queue <int> targetPositions;
	CalculateInterfaceToBufferSetupConfig(sourceChunks, targetPositions, ANY_CHUNK, ANY_POSITION);
	SetCrossbarSetupDataForStream(sourceChunks, targetPositions, outputStreamSetupData);
}

void DMACrossbarSetup::CalculateInterfaceToBufferSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION)
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

void DMACrossbarSetup::FindInputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& inputStreamSetupData)
{
	std::queue <int> sourceChunks;
	std::queue <int> targetPositions;
	CalculateBufferToInterfaceSetupConfig(sourceChunks, targetPositions, ANY_CHUNK, ANY_POSITION);
	SetCrossbarSetupDataForStream(sourceChunks, targetPositions, inputStreamSetupData);
}

void DMACrossbarSetup::CalculateBufferToInterfaceSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION)
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

void DMACrossbarSetup::SetCrossbarSetupDataForStream(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, DMASetupData& streamSetupData)
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