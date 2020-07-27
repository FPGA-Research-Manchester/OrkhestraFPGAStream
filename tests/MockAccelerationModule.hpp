#pragma once
#include "AccelerationModule.hpp"
#include <cstdint>

class MockAccelerationModule : public AccelerationModule {
public:
	MockAccelerationModule(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition);
	~MockAccelerationModule() override;
	void writeToModule(uint32_t moduleInternalAddress, uint32_t writeData);
	auto readFromModule(uint32_t moduleInternalAddress) -> uint32_t;
};