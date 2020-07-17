#pragma once
#include "DMASetupData.hpp"
#include <queue>
class DMACrossbarSetup
{
public:
	void FindInputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& inputStreamSetupData);
	void FindOutputCrossbarSetupData(const int& ANY_CHUNK, const int& ANY_POSITION, DMASetupData& outputStreamSetupData);
private:
	void CalculateInterfaceToBufferSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION);
	void CalculateBufferToInterfaceSetupConfig(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, const int& ANY_CHUNK, const int& ANY_POSITION);
	void SetCrossbarSetupDataForStream(std::queue<int>& sourceChunks, std::queue<int>& targetPositions, DMASetupData& streamSetupData);
};
