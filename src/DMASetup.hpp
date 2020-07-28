#pragma once
#include <vector>

#include "DMA.hpp"
#include "DMASetupData.hpp"
class DMASetup {
 public:
  void SetupDMAModule(DMAInterface& dmaEngine, std::vector<int>& dbData,
                      int recordSize, int recordCount, int inputStreamID,
                      int outputStreamID);

 private:
  void WriteSetupDataToDMAModule(std::vector<DMASetupData>& setupDataForDMA,
                                 DMAInterface& dmaEngine);
  void SetUpDMAIOStreams(DMASetupData& streamSetupData,
                         DMAInterface& dmaEngine);
  void SetUpDMACrossbars(DMASetupData& streamSetupData,
                         DMAInterface& dmaEngine);
  void CalculateDMAStreamSetupData(DMASetupData& streamSetupData,
                                   const int& maxChunkSize,
                                   const int& maxDDRBurstSize,
                                   const int& maxDDRSizePerCycle,
                                   std::vector<int>& dbData, int recordSize);
};
