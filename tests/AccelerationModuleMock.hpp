#pragma once
#include <cstdint>
#include "AccelerationModule.hpp"

class AccelerationModuleMock : public AccelerationModule {
public:
	AccelerationModuleMock(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition);
	~AccelerationModuleMock();
	void writeToModule(uint32_t moduleInternalAddress, uint32_t writeData);
	uint32_t readFromModule(uint32_t moduleInternalAddress);
};