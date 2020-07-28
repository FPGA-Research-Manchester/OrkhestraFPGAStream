#pragma once
#include <vector>

#include "DMA.hpp"
#include "DMASetupData.hpp"
class DMASetup {
 public:
  static void SetupDMAModule(DMAInterface& dmaEngine, std::vector<int>& dbData,
                             int recordSize, int recordCount, int inputStreamID,
                             int outputStreamID);

 private:
  static void WriteSetupDataToDMAModule(
      std::vector<DMASetupData>& setupDataForDMA, DMAInterface& dmaEngine);
  static void SetUpDMAIOStreams(DMASetupData& streamSetupData,
                                DMAInterface& dmaEngine);
  static void SetUpDMACrossbars(DMASetupData& streamSetupData,
                                DMAInterface& dmaEngine);
  static void CalculateDMAStreamSetupData(DMASetupData& streamSetupData,
                                          const int& maxChunkSize,
                                          const int& maxDDRBurstSize,
                                          const int& maxDDRSizePerCycle,
                                          std::vector<int>& dbData,
                                          int recordSize);
};
