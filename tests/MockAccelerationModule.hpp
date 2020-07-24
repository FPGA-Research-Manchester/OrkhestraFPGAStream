#pragma once
#include <cstdint>
#include "AccelerationModule.hpp"

class MockAccelerationModule : public AccelerationModule {
public:
	MockAccelerationModule(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition);
	~MockAccelerationModule();
	void writeToModule(uint32_t moduleInternalAddress, uint32_t writeData);
	uint32_t readFromModule(uint32_t moduleInternalAddress);
};