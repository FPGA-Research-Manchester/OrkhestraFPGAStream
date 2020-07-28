#pragma once
#include <queue>

#include "DMASetupData.hpp"
class DMACrossbarSetup {
 public:
  static void FindInputCrossbarSetupData(const int& ANY_CHUNK,
                                         const int& ANY_POSITION,
                                         DMASetupData& inputStreamSetupData);
  static void FindOutputCrossbarSetupData(const int& ANY_CHUNK,
                                          const int& ANY_POSITION,
                                          DMASetupData& outputStreamSetupData);

 private:
  static void CalculateInterfaceToBufferSetupConfig(
      std::queue<int>& sourceChunks, std::queue<int>& targetPositions,
      const int& ANY_CHUNK, const int& ANY_POSITION);
  static void CalculateBufferToInterfaceSetupConfig(
      std::queue<int>& sourceChunks, std::queue<int>& targetPositions,
      const int& ANY_CHUNK, const int& ANY_POSITION);
  static void SetCrossbarSetupDataForStream(std::queue<int>& sourceChunks,
                                            std::queue<int>& targetPositions,
                                            DMASetupData& streamSetupData);
};
