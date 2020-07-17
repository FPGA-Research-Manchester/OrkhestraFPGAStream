#pragma once
#include <vector>
#include "DMA.hpp"
#include "DMASetupData.hpp"
class DMASetup
{
public:
	void SetupDMAModule(int recordCount, std::vector<int>& dbData, int recordSize, DMA& dmaEngine);
private:
	void WriteSetupDataToDMAModule(std::vector<DMASetupData>& setupDataForDMA, DMA& dmaEngine);
	void SetUpDMAIOStreams(DMASetupData& streamSetupData, DMA& dmaEngine);
	void SetUpDMACrossbars(DMASetupData& streamSetupData, DMA& dmaEngine);
	void CalculateDMAStreamSetupData(DMASetupData& streamSetupData, const int& maxChunkSize, const int& maxDDRBurstSize, const int& maxDDRSizePerCycle, std::vector<int>& dbData, int recordSize);
};
