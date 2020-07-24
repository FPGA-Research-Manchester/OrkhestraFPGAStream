#include "MockAccelerationModule.hpp"
#include <cstdint>

MockAccelerationModule::MockAccelerationModule(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition):AccelerationModule(ctrlAXIbaseAddress, modulePosition){}
MockAccelerationModule::~MockAccelerationModule(){}

void MockAccelerationModule::writeToModule(uint32_t moduleInternalAddress, uint32_t writeData)
{
	AccelerationModule::writeToModule(moduleInternalAddress, writeData);
}

uint32_t MockAccelerationModule::readFromModule(uint32_t moduleInternalAddress)
{
	return AccelerationModule::readFromModule(moduleInternalAddress);
}
