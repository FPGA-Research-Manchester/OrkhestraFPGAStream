#pragma once
#include <vector>
#include <queue>
#include "DMA.hpp"
#include "DMASetupData.hpp"
class DMASetup
{
public:
	void SetupDMAModule(int recordCount, std::vector<int>& dbData, int recordSize, DMA& dmaEngine);
private:
	void FindOutputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& outputStreamSetupData);
	void CalculateInterfaceToBufferSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION);
	void FindInputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& inputStreamSetupData);
	void CalculateBufferToInterfaceSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION);

	void WriteSetupDataToDMAModule(std::vector<DMASetupData>& setupDataForDMA, DMA& dmaEngine);
	void SetUpDMACrossbars(DMASetupData& streamSetupData, DMA& dmaEngine);
	void SetCrossbarSetupDataForStream(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, DMASetupData& streamSetupData);
	void SetUpDMAIOStreams(DMASetupData& streamSetupData, DMA& dmaEngine);
	void CalculateDMAStreamSetupData(DMASetupData& streamSetupData, const int& maxChunkSize, const int& maxDDRBurstSize, const int& maxDDRSizePerCycle, std::vector<int>& dbData, int recordSize);
};
