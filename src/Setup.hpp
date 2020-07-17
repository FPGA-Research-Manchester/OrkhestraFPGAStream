#pragma once
#include <vector>

class Setup
{
public:
	void SetupQueryAcceleration(int* volatile& memoryPointer, std::vector<int>& dbData, int recordCount, int recordSize);
};
