#pragma once
#include <cstdio>
#include <tuple>
#include <vector>

#include "DMACrossbarSetupData.hpp"
struct DMASetupData {
  DMASetupData() : crossbarSetupData(32) {}
  int streamID, DDRBurstLength, recordsPerDDRBurst, recordCount,
      chunksPerRecord, bufferStart, bufferEnd;
  uintptr_t streamAddress;
  std::vector<std::tuple<int, int>> recordChunkIDs;
  std::vector<DMACrossbarSetupData> crossbarSetupData;
  bool isInputStream;
};