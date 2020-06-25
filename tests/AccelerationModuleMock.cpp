#include "AccelerationModuleMock.hpp"
#include <cstdint>

AccelerationModuleMock::AccelerationModuleMock(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition):AccelerationModule(ctrlAXIbaseAddress, modulePosition){}
AccelerationModuleMock::~AccelerationModuleMock(){}

void AccelerationModuleMock::writeToModule(uint32_t moduleInternalAddress, uint32_t writeData)
{
	AccelerationModule::writeToModule(moduleInternalAddress, writeData);
}

uint32_t AccelerationModuleMock::readFromModule(uint32_t moduleInternalAddress)
{
	return AccelerationModule::readFromModule(moduleInternalAddress);
}
