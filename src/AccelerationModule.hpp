#pragma once
#include <cstdint>
class AccelerationModule
{
private:
	int* volatile const controlAXIbaseAddress_;
	const uint32_t modulePosition_;

	int* volatile calculateMemoryMappedAddress(uint32_t moduleInternalAddress);
protected:
	void writeToModule(uint32_t moduleInternalAddress, uint32_t writeData);
	uint32_t readFromModule(uint32_t moduleInternalAddress);
	AccelerationModule(int* volatile ctrlAXIbaseAddress, uint32_t modulePosition);
public:
	virtual ~AccelerationModule() = 0;
};